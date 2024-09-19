//****************************************************************
//
// Upgrade.cpp : auto-upgrade related routines for client end
//
//****************************************************************

#ifdef HORATIO
	#define DISP 0
#else
	#define DISP 0
#endif

#include "stdafx.h"
#include <wininet.h>
#include <winsock.h>
#include <process.h>
#include <direct.h>
#include <sys\stat.h>
#include "resource.h"

#define UPGRADE_FILE_DIR	".\\temp\\"
//#define UPGRADE_FILE_DIR	""
#define EXTRA_FREE_SPACE	(3<<20)	// make sure there's an extra 'n'MB available.

int AutoUpgradeDisable;	// set if we should not act on upgrades

char *UpgradeMessageTitle = "e-Media Poker.com Auto-upgrader";
char *UpgradeStatusWindowPosName = "Download Status Window Position";
HWND hUpgradeStatusDlg;

static WORD32 Download_Start_Time;		// SecondCounter when we started the download
static WORD32 DownloadBytes_total;
static WORD32 DownloadBytes_completed;
static int iCancelUpgrade;		// set when 'CANCEL' is pressed.
static int iNotifyIfUpToDate;	// set if we should notify the user that they are up-to-date

static int iDisableInternetFunctions;	// set if we should disable use of the internet functions (for WriteFileFromUrl())

//*********************************************************
// 1999/06/22 - MB
//
// Update the download bytes and total files, ETA, etc. on
// the dialog box.
//
static void UpdateByteProgress(void)
{
	if (hUpgradeStatusDlg) {
		// Calculate a KB/s rate.
		WORD32 elapsed = SecondCounter - Download_Start_Time;
		if (!elapsed) elapsed = 1;	// never allow zero.
		WORD32 throughput = DownloadBytes_completed / elapsed;
		char msg[100];
		if (DownloadBytes_total) {
			sprintf(msg, "%u of %u KB downloaded. %.1fKB/sec",
					DownloadBytes_completed/1024, DownloadBytes_total/1024, (float)throughput / 1024.0);
		} else {
			sprintf(msg, "%u bytes downloaded. %.1fKB/sec",
					DownloadBytes_completed, (float)throughput / 1024.0);
		}
		SetWindowTextIfNecessary(GetDlgItem(hUpgradeStatusDlg, IDC_BYTES_TO_GO), msg);

		int percentage = 0;
		if (DownloadBytes_total) {
			percentage = DownloadBytes_completed*100 / DownloadBytes_total;
		}
		PostMessage(GetDlgItem(hUpgradeStatusDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM)percentage, 0);
	}
}

//*********************************************************
// 1999/06/22 - MB
//
// Set the IDC_CURRENT_FILE entry in the status dialog box
//
void SetCurrentFileMsg(char *operation, char *full_pathname)
{
	if (hUpgradeStatusDlg) {
		char msg[MAX_FNAME_LEN+50], basename[MAX_FNAME_LEN];
		GetNameFromPath(full_pathname, basename);
		sprintf(msg, "%s%s", operation, basename);
		SetWindowTextIfNecessary(GetDlgItem(hUpgradeStatusDlg, IDC_CURRENT_FILE), msg);
	}
}


//*********************************************************
// 17-07-20003 cris
//
// Set the IDC_TASKS label
//
void upgradeTasks(char *operation)
{
	if (hUpgradeStatusDlg) {
		char msg[MAX_FNAME_LEN+50];
		sprintf(msg, "%s", operation);
		SetWindowTextIfNecessary(GetDlgItem(hUpgradeStatusDlg, IDC_TASKS), msg);
	}
}

//*********************************************************
// 1999/10/28 - MB
//
// Grab a file from a URL and write it to a specific disk file
// using Microsoft's Internet functions.  They will use a proxy if
// necessary and might even cache things.
//
ErrorType WriteFileFromUrlUsingInternetFunctions(HINTERNET inet_hndl, char *source_url, char *dest_path)
{
	ErrorType err = ERR_ERROR;
	HINTERNET hi = InternetOpenUrl(inet_hndl, source_url,
			NULL, 0, INTERNET_FLAG_HYPERLINK
		  #if 1	//19990715MB: force all caches to be disabled?
			| INTERNET_FLAG_RELOAD
			| INTERNET_FLAG_NO_CACHE_WRITE
			| INTERNET_FLAG_PRAGMA_NOCACHE 
		  #endif
			, 0);
	if (hi) {
		FILE *fd = fopen(dest_path, "wb");
		if (fd) {
			// It opened ok... read it in...
			#define BUFFER_SIZE 2048	// about .5s worth on a 56K modem
			void *ptr = malloc(BUFFER_SIZE);
			if (ptr) {
				DWORD total_bytes_read = 0;
				forever {
					DWORD bytes_read = 0;
					BOOL result = InternetReadFile(hi, (LPVOID)ptr, BUFFER_SIZE, &bytes_read);
					kp(("%s(%d) InternetReadFile('%s'): success=%d, bytes_read = %d\n",_FL, source_url, result, bytes_read));
					if (result) {
						// We got some bytes.
						if (bytes_read==0) {	// EOF?
							err = ERR_NONE;
							if (total_bytes_read==0) {
								// Hmmm... we didn't read ANYTHING for this file.
								// that can't be good.  Return an error message.
								Error(ERR_ERROR, "%s(%d) InternetReadFile('%s') didn't read any bytes.\n",_FL,source_url);
								err = ERR_ERROR;
							}
							break;
						}
						size_t bytes_written = fwrite(ptr, 1, bytes_read, fd);
						DownloadBytes_completed += bytes_written;
						total_bytes_read += bytes_written;
						UpdateByteProgress();
						if (bytes_written != bytes_read) {
							// Not everything got written.
							// For now, assume a disk full error.
							Error(ERR_ERROR, "%s(%d) InternetReadFile('%s') couldn't write all bytes to disk. Disk full?",_FL,source_url);
							err = ERR_ERROR;
							break;
						}
					} else {
						Error(ERR_ERROR, "%s(%d) InternetReadFile('%s') failed.  Error = %d\n",_FL,source_url,GetLastError());
						err = ERR_ERROR;
						break;
					}
					if (iCancelUpgrade || ExitNowFlag) {
						break;
					}
				}
				free(ptr);
			}
			fclose(fd);
		}
		InternetCloseHandle(hi);
		hi = 0;
	} else {
		Error(ERR_ERROR, "%s(%d) InternetOpenUrl('%s') failed.  Error = %d",_FL,source_url,GetLastError());
		err = ERR_ERROR;
	}
	return err;
}

//*********************************************************
// 1999/10/28 - MB
//
// Callback function while writing a url file to disk
// Returns non-zero to cancel.
//
static int WriteFileFromURLCallback(WORD32 additional_bytes_received_and_written)
{
	if (additional_bytes_received_and_written) {
		DownloadBytes_completed += additional_bytes_received_and_written;
		UpdateByteProgress();
	}
	if (iCancelUpgrade || ExitNowFlag) {
		return 1;	// cancel
	}
	return 0;	// don't cancel.
}

//*********************************************************
// 1999/05/23 - MB
//
// Grab a file from a URL and write it to a specific disk file.
// Default to using Microsoft's internet functions, but fall back
// to our own code if that fails.
//
ErrorType WriteFileFromUrl(HINTERNET inet_hndl, char *source_url, char *dest_path)
{
	FixUrl(source_url);	// make all slashes forward slashes (url format)
	SetCurrentFileMsg("Retrieving: ", source_url);
	ErrorType result = ERR_ERROR;
	// retrieve_method: 0=internet functions, 1=direct socket to port 80, 2=direct socket to port 26003
	static int retrieve_method = 1;	// default to method 0
	// If we have succeeded with the internet functions or if the socket functions
	// also failed, try again with the internet functions.
	long start_seconds = SecondCounter;
  #if 0
	kp1((ANSI_WHITE_ON_GREEN"%s(%d) WARNING: INTERNET FUNCTIONS ARE NOT EVEN BEING ATTEMPTED! TURN THEM BACK ON.\n",_FL));
	iDisableInternetFunctions = TRUE;
  #endif

	int attempts = 0;
	do {
		// try to fetch a file based on the current retrieve method.
		if (iDisableInternetFunctions && retrieve_method==0) {
			retrieve_method++;	// skip over internet functions for testing purposes.
		}

		attempts++;

		pr(("%s(%d) attempt #%d, method %d, getting file '%s' and saving as '%s'\n",
				_FL, attempts, retrieve_method, source_url, dest_path));
		switch (retrieve_method) {
		case 0:	// use our socket functions to port 26003
			result = WriteFileFromUrlUsingSockets(source_url, dest_path, WriteFileFromURLCallback, 26003);
			pr(("%s(%d) result = %d\n", _FL, result));
			if (result != ERR_NONE && !iCancelUpgrade && !ExitNowFlag) {
				retrieve_method++;	// try another method.
				Error(ERR_NOTE, "%s(%d) WriteFileFromUrlUsingSockets('%s') (port 26003) failed (err=%d, elapsed=%ds).",_FL, source_url, result, SecondCounter - start_seconds);
			}
			//Error(ERR_NOTE, "%s(%d) WriteFileFromUrlUsingSockets('%s') (port 26003) connected 0 (err=%d, elapsed=%ds).",_FL, source_url, result, SecondCounter - start_seconds);
			break;
		case 1:	// use our socket functions to port 80
			result = WriteFileFromUrlUsingSockets(source_url, dest_path, WriteFileFromURLCallback);
			if (result != ERR_NONE && !iCancelUpgrade && !ExitNowFlag) {
				retrieve_method++;	// try another method.
				Error(ERR_NOTE, "%s(%d) WriteFileFromUrlUsingSockets('%s') failed (err=%d, elapsed=%ds).",_FL, source_url, result, SecondCounter - start_seconds);
			}
			//Error(ERR_NOTE, "%s(%d) WriteFileFromUrlUsingSockets('%s') connected 1 (err=%d, elapsed=%ds).",_FL, source_url, result, SecondCounter - start_seconds);
			break;
		case 2:	// use the Windows Internet functions
			result = WriteFileFromUrlUsingInternetFunctions(inet_hndl, source_url, dest_path);
			if (result != ERR_NONE && !iCancelUpgrade && !ExitNowFlag) {
				retrieve_method++;	// try another method.
				Error(ERR_NOTE, "%s(%d) WriteFileFromUrlUsingInternetFunctions('%s') failed (err=%d, elapsed=%ds)",_FL, source_url, result, SecondCounter - start_seconds);
			}
			//Error(ERR_NOTE, "%s(%d) WriteFileFromUrlUsingInternetFunctions('%s') connected 2 (err=%d, elapsed=%ds)",_FL, source_url, result, SecondCounter - start_seconds);
			break;
		}
		if (retrieve_method > 2) {	// ran out of methods?
			retrieve_method = 0;	// wrap to first method.
		}
	} while(!ExitNowFlag && result!=ERR_NONE && attempts<6 && !iCancelUpgrade && !ExitNowFlag);

	if (attempts > 1 && result==ERR_NONE) {
		Error(ERR_NOTE, "%s(%d) File '%s' successfully retrieved using method %d", _FL, source_url, retrieve_method);
	}

	if (result != ERR_NONE && !iCancelUpgrade && !ExitNowFlag) {
		Error(ERR_ERROR, "%s(%d) Cannot retrieve file '%s' using any method.", _FL, source_url);
	}

	return result;
}

//*********************************************************
// 1999/05/23 - MB
//
// Create a temporary destination name given a URL.
// The resulting filename looks like this: ".\upgrtemp\filename.ext".
// The dest_name array should be of size MAX_FNAME_LEN or larger.
//
void GetTempFileNameFromUrl(char *source_url, char *dest_name)
{
	strcpy(dest_name, UPGRADE_FILE_DIR);
	GetNameFromPath(source_url, dest_name+strlen(dest_name));
	_mkdir(UPGRADE_FILE_DIR);	// try to create it just in case it's not there.
}

//*********************************************************
// 1999/05/23 - MB
//
// Grab a file from a URL and write it to a file with the same name
// in our local temp directory (".\upgrtemp\filename.ext")
//
ErrorType WriteTempFileFromUrl(HINTERNET inet_hndl, char *source_url)
{
	char dest_name[MAX_FNAME_LEN];
	GetTempFileNameFromUrl(source_url, dest_name);
	return WriteFileFromUrl(inet_hndl, source_url, dest_name);
}

//*********************************************************
// 1999/05/25 - MB
//
// Read the upgrade info file (text) into a structure.
//
// The resulting structure must be freed with free().
//
// note: these structures are defined in both
//	upgrader\upgrader.cpp and client\upgrade.cpp
//
struct FileUpgradeInfo {
	ULONG length;				// unpacked length
	ULONG compressed_length;	// packed length
	time_t date;				// date of file.
	int download_needed;		// set if we need to download this file.
	int copy_needed;			// set if it needs to be copied to main dir during install
	WORD32 crc;					// crc of file.
	char name[MAX_FNAME_LEN];	// filename (no path)
};
struct UpgradeInfo  {
	WORD32 version;
	int file_count;
	long total_structure_len;	// length of this structure.
	struct FileUpgradeInfo files[1];	// variable array size (uses file_count)
};

struct UpgradeInfo *ReadUpgradeInfoFile(char *src_file_name)
{
	FILE *fd = fopen(src_file_name, "rt");
	if (!fd) {
		Error(ERR_ERROR, "%s(%d) could not read upgrade file '%s'", _FL, src_file_name);
		return NULL;
	}

	// First, make a pass to count the number of files.
	#define MAX_LINE_LEN	300
	char str[MAX_LINE_LEN];
	int max_file_count = 0;
	forever {
		char *result = fgets(str, MAX_LINE_LEN, fd);
		if (!result) {
			break;	// all done.
		}
		max_file_count++;
	}
	//kp(("%s(%d) max_file_count = %d\n", _FL, max_file_count));

	// allocate space for the files.
	long len = max_file_count * sizeof(struct FileUpgradeInfo) + sizeof(struct UpgradeInfo);
	struct UpgradeInfo *ui = (struct UpgradeInfo *)malloc(len);
	if (!ui) {
		Error(ERR_ERROR, "%s(%d) Could not malloc(%d) for upgrade info.",_FL,len);
		fclose(fd);
		return NULL;
	}
	memset(ui, 0, len);	// always clear to zero.
	ui->total_structure_len = len;

	// Now read the file again and store the results into the structure.
	fseek(fd, 0, SEEK_SET);
	// First, retrieve the version number.
	char *result = fgets(str, MAX_LINE_LEN, fd);
	if (result) {
		UINT major, minor, build;
		major = minor = build = 0;
		sscanf(str, "%d.%d.%d", &major, &minor, &build);
		ui->version = (major<<24) | (minor<<16) | build;
	}
	// Now retrieve each file...
	while (ui->file_count < max_file_count) {
		char *result = fgets(str, MAX_LINE_LEN, fd);
		if (!result) {
			break;	// all done.
		}
		// Locate the first ','... it marks the end of the filename.
		char *comma_ptr = strchr(str, ',');
		if (comma_ptr) {
			*comma_ptr++ = 0;
			strnncpy(ui->files[ui->file_count].name, str, MAX_FNAME_LEN);
			sscanf(comma_ptr, "%u,%u,%u,%u",
				&ui->files[ui->file_count].length,
				&ui->files[ui->file_count].compressed_length,
				&ui->files[ui->file_count].date,
				&ui->files[ui->file_count].crc);
			ui->file_count++;
		}
	}

	fclose(fd);

	return ui;
}

//*********************************************************
// 1999/07/15 - MB
//
// Check if a particular file exists and matches the one from the
// upgrade info file.
// returns: match flag (TRUE=matches, FALSE=not found or does not match)
//
int CheckForMatchingFile(char *fname, struct FileUpgradeInfo *fui)
{
	int matched = FALSE;
	struct stat s;
	zstruct(s);
	if (!stat(fname, &s)) {
		// found it.
		// We got the info on the file... compare date and length
		pr(("%s(%d) ... found.  existing file is %d bytes, date = %u (want %u)\n",_FL,s.st_size, s.st_mtime,fui->date));
		// Make sure the times are within a few seconds of each other.
		if (s.st_size != (int)fui->length) {
			pr(("%s(%d) size for files don't match\n", _FL));
	  #if 0	//19990921MB: may have time zone problems.
		} else if (abs((long)(s.st_mtime - fui->date)) >= 5) {
			pr(("%s(%d) time for files don't match\n", _FL));
	  #endif
		} else {
			// Everything else matches... check the CRC
			WORD32 crc = CalcCRC32forFile(fname);
			if (crc==fui->crc) {
				pr(("%s(%d) CRC's for file '%s' match\n",_FL,fname));
				matched = TRUE;
			} else {
				pr(("%s(%d) CRC's for file '%s' DON'T match\n",_FL,fname));
			}
		}
	}
	return matched;
}	

//*********************************************************
// 1999/05/25 - MB
//
// Go through the file list and determine which files need
// to be downloaded.
// Returns the total # of bytes which need to be downloaded and
// the total disk space needed.
//
void DetermineDownloadFiles(struct UpgradeInfo *upgr_info, ULONG *output_download_total, ULONG *output_diskspace_total, int *no_work_flag)
{
	ULONG download_total = 0;
	ULONG diskspace_total = 0;
	*no_work_flag = TRUE;	// default to assuming no work needs to be done.
	struct FileUpgradeInfo *fui = upgr_info->files;
	for (int i=0 ; i<upgr_info->file_count ; i++, fui++) {
		pr(("%s(%d) Checking '%s'...\n", _FL, fui->name));
		// Check this file's date and length.
		fui->download_needed = FALSE;
		fui->copy_needed = FALSE;
		char fname[MAX_FNAME_LEN];

		// First, check the main directory and the download directory to
		// see if we've already downloaded the latest version...
		strcpy(fname, fui->name);
		if (!CheckForMatchingFile(fname, fui)) {
			// Not found or did not match... check the upgrade directory...
			*no_work_flag = FALSE;	// at least one file needs copying or downloading
			strcpy(fname, UPGRADE_FILE_DIR);
			strcat(fname, fui->name);
			if (!CheckForMatchingFile(fname, fui)) {
				// Not found or did not match... it needs downloading.
				fui->download_needed = TRUE;
			} else {
				// It matched... it just needs copying
				fui->copy_needed = TRUE;
			}
		}
		if (fui->download_needed) {
			pr(("%s(%d) %s needs downloading (%d bytes).  Adding %d bytes to diskspace total.\n", _FL, fui->name, fui->compressed_length, fui->length));
			diskspace_total += fui->length;
			download_total += fui->compressed_length;
			fui->copy_needed = TRUE;
		}
	}
	if (output_download_total)
		*output_download_total = download_total;
	if (output_diskspace_total)
		*output_diskspace_total = diskspace_total;
}

//*********************************************************
// 1999/09/10 - MB
//
// Show the 'upgrade failed' message box.
//
void ShowUpgradeFailedMessageBox(void)
{
	MessageBox(NULL, "Downloading the upgrade failed for some reason.\n\n"

					 "There might be a proxy (caching) problem (most likely),\n"
					 "your hard disk might be full (not very likely), or\n"
					 "there may be a problem getting the data from the server.\n\n"

					 "The upgrade will not be installed at this time.\n\n"

					 "One option at this point is to try downloading the complete\n"
					 "version from our web site and re-installing.  Please be sure\n"
					 "to make a note of your player ID and password beforehand.\n\n"

					 "Select 'Check For Update' from the help menu to try again.",
				UpgradeMessageTitle, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
}

//*********************************************************
// Read the upgrade file header
//  Cris 17/07/2003
void readUpgradeFileHeader(FILE* f, struct upgrade_rules_header *uh){
	fread(uh,sizeof(upgrade_rules_header),1,f);
};//readUpgradeFileHeader


//*********************************************************
// Read a rule in the upgrade in the 
//Upgrade file
//  Cris 17/07/2003
void readRuleUpgradeFile(FILE* f, struct upgrade_rules *uh){
	fread(uh,sizeof(upgrade_rules),1,f);
};//readUpgradeFileHeader

//*********************************************************
//Download File
//cris  17/07/2003
void downloadFile(HINTERNET inet_hndl,struct upgrade_rules ur){
	ErrorType err = ERR_NONE;
	DownloadBytes_total=ur.bytes ;
	err = WriteFileFromUrl(inet_hndl, ur.source , ur.target);
	
};//downlaodFile

//***********************************************************
//Process a upgrade rule
///Cris 17/07/20003
int processUpgradeRule(struct upgrade_rules ur){
    switch (ur.type ) {
		case 0:
           			
			break;
		case 1:
			
			break;
		case 2:
			
			break;
		case 3:
			
			break;
		case 4:
			
			break;
		case 5:
			
			break;
		case 6:
			
			break;
		case 7:
			
			break;
		case 8:
			
			break;
		case 9:
			
			break;
		case 10:
			
			break;
		case 11:
			
			break;

	};//switch
	return 0;
};//processUpgradeRule

//*********************************************************
// 1999/05/23 - MB
//
// Entry point for the auto-upgrade thread.
//
void _cdecl UpgradeThreadEntryPoint(void *args)
{
  #if INCL_STACK_CRAWL
	volatile int top_of_stack_signature = TOP_OF_STACK_SIGNATURE;	// for stack crawl
  #endif
	//19990806MB: never allow two upgrade threads to start.
	// Anyone using a build prior to 08/06 might get more than one 'upgrade downloaded'
	// dialog box after the upgrade has been downloaded.
	static int upgrade_thread_started;
	if (upgrade_thread_started) {
		return;	// never allow it to start twice.
	}
	upgrade_thread_started = TRUE;
	iCancelUpgrade = FALSE;

	HINTERNET inet_hndl = InternetOpen(szAppName,
			INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	//kp(("%s(%d) inet_hndl = %d\n", _FL, inet_hndl));
	if (!inet_hndl) {
		Error(ERR_ERROR, "%s(%d) InternetOpen() failed.  Error = %d\n",_FL,GetLastError());
		return;
	}
	int retried_count = 0;	// # of times we've retried so far...

startover:
	ErrorType err = ERR_NONE;
	pr(("%s(%d) Upgrade thread has now started.\n",_FL));
	// Here's what we've got to work with:
	//  ServerVersionInfo contains info from server about new versions.
	//  ClientVersionNumber contains info about our program.

	// If a new version is available, the first step is to gather
	// some information about it before prompting the user.
	// We want to estimate how large it will be.
	Download_Start_Time = SecondCounter;
	DownloadBytes_total = 0;
	DownloadBytes_completed = 0;

	if (iCancelUpgrade || ExitNowFlag) {
		//goto exitupgrade;
	}

	char upgrade_info_file[MAX_FNAME_LEN];

	//kp(("%s(%d) ServerVersionInfo.new_ver_auto_url = '%s'\n", _FL, ServerVersionInfo.new_ver_auto_url));
	DownloadBytes_total=169;
  //  strcpy(ServerVersionInfo.new_ver_auto_url,"http://200.9.37.164/upgrade/var.tar.gz");
//	strcpy(upgrade_info_file,"var.tar.gz");
	//MessageBox(NULL, upgrade_info_file, "Archivo",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
	GetTempFileNameFromUrl(ServerVersionInfo.new_ver_auto_url, upgrade_info_file);
//	MessageBox(NULL, ServerVersionInfo.new_ver_auto_url, "URL",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
	
	err = WriteFileFromUrl(inet_hndl, ServerVersionInfo.new_ver_auto_url, upgrade_info_file);
	//MessageBox(NULL, upgrade_info_file, "Archivo",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
	if (iCancelUpgrade || ExitNowFlag) {
		//goto exitupgrade;
	}
	if (err==ERR_NONE) {
		
		// We've got the info file... do something with it.
		//kp(("%s(%d) We've successfully retrieved the file '%s'\n", _FL, upgrade_info_file));
		// Uncompress the downloaded file if necessary.
		{
			//MessageBox(NULL, "No error", "????",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
			char ext[MAX_FNAME_LEN];
			GetExtension(upgrade_info_file, ext);
			if (!strcmp(ext, "gz")) {
				// It's a .gz file... uncompress it.
				SetCurrentFileMsg("Unpacking: ", upgrade_info_file);

				err = gunzipfile(upgrade_info_file);
				if (err==ERR_NONE) {
					// No error uncompressing... remove the .gz version.
					remove(upgrade_info_file);
				} else {
					SetCurrentFileMsg("Incomplete.", "");
					Error(ERR_ERROR, "unpacking %s failed.", upgrade_info_file);
					ShowUpgradeFailedMessageBox();
					//goto exitupgrade;
				};//if
				TrimExtension(upgrade_info_file);	// remove the .gz
			};//if
		}

		// Read the upgrade info file into memory.
	//	struct UpgradeInfo *upgr_info;
		struct upgrade_rules_header header;
		struct upgrade_rules rule;
		char strTemp[200];
		FILE* upgradeFile;
		upgradeFile=fopen(upgrade_info_file,"rt");
		if(upgradeFile==NULL){
		   MessageBox(NULL, "Upgrade rules file can't open.", "Error",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);		 
           //goto exitupgrade;
		};//if
		zstruct(header);
		readUpgradeFileHeader(upgradeFile,&header);
		sprintf(strTemp, "%d", header.num_rules);
		upgradeTasks (strTemp);   
	//	MessageBox(NULL, strTemp , "Rules",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);		 
        //upgradeTasks (strTemp);  
		//lee las reglas
		//strcpy(strTemp,"Step")
		if (header.num_rules){
			for (int i=1;i<=header.num_rules ;i++){
				//read a rule
	            sprintf(strTemp, "Step %d of %d", i,header.num_rules); 		
				upgradeTasks (strTemp);
				zstruct(rule);
				readRuleUpgradeFile(upgradeFile,&rule);
				//************************************
				if (rule.type ==1){
					//baja el archivo 
					DownloadBytes_total=rule.bytes ;
					GetTempFileNameFromUrl(rule.source , strTemp);
                    err = WriteFileFromUrl(inet_hndl, rule.source , strTemp);
					char ext[MAX_FNAME_LEN];
					GetExtension(strTemp, ext);
					
					if (!strcmp(ext, "gz")) {
						// It's a .gz file... uncompress it.
						SetCurrentFileMsg("Unpacking: ", strTemp);

						err = gunzipfile(strTemp);
						if (err==ERR_NONE) {
							// No error uncompressing... remove the .gz version.
							remove(strTemp);
						} else {
							SetCurrentFileMsg("Incomplete.", "");
							Error(ERR_ERROR, "unpacking %s failed.", strTemp);
							ShowUpgradeFailedMessageBox();
							//goto exitupgrade;
						};//if
						TrimExtension(strTemp);	// remove the .gz
					};//if
                    
				};//if
				//***********************************
				
				sprintf(strTemp, "Rule Type: %d Source: %s Target: %s Byte: %d ",rule.type,rule.source,rule.target,rule.bytes  ); 
				//MessageBox(NULL, strTemp , "Rules",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
			};//for
		};//if
//		MessageBox(NULL, "LLegue al final", "F I N A L ",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
		//cierra el archivo
        if(upgradeFile){
			fclose(upgradeFile);
        };//
		ShowWindow(hUpgradeStatusDlg, SW_HIDE);
//cris 23-1-2004
#if 0
//end cris 23-1-2004
        //ejecuta el upgrader.exe
		int res=MessageBox(hCardRoomDlg, "A new software version is ready. \n Please click yes to update your files, \n If you click yes the application will close temporarily \n after we download the files, any open games will be closed.", "e-Media Poker Upgrader",MB_YESNO);
		if(res==IDYES){
		       PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
		       //WinExec("upgrade.bat", SW_SHOW);
		       system("upgrade.bat");
		}else{
			//save the stata of upgrade
	       // Defaults.upgrade=1;
			//WriteDefaults(true);
			
		};//if
		//cierra la aplizacion actual						
		//PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
//cris 23-1-2004
#endif
//end cris 23-1-2004

//cris 23-1-2004
		//Force never ask to the user about upgrade process
		PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
		system("upgrade.bat");
//end cris 23-1-2004
	}
#if 0
	/*	SetCurrentFileMsg("Processing: ", upgrade_info_file);
		upgr_info = ReadUpgradeInfoFile(upgrade_info_file);
		pr(("%s(%d) There are %d files in the new version.\n", _FL, upgr_info->file_count));

		// Go through the file list and determine which files need
		// to be downloaded.
		ULONG total_download, total_diskspace;
		int no_work_flag;
		DetermineDownloadFiles(upgr_info, &total_download, &total_diskspace, &no_work_flag);
		if (no_work_flag) {
			if (iNotifyIfUpToDate) {
				if (hUpgradeStatusDlg) {
					PostMessage(hUpgradeStatusDlg, WMP_CLOSE_YOURSELF, 0, 0);
				};//if (hUpgradeStatusDlg)
				if (upgr_info->file_count) {
					MessageBox(NULL,
						"You have the latest version.\n\n"
						"All files have been verified to be\n"
						"the correct version.\n\n"
						"No update necessary.",
						UpgradeMessageTitle, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
				} else {
					MessageBox(NULL, "Cannot retrieve upgrade info.", UpgradeMessageTitle, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
				};//if upgr_info->file_count
			};//if (iNotifyIfUpToDate) 
			pr(("%s(%d) There are no new files to be downloaded or copied; exiting the upgrade thread.\n", _FL));
			goto exitupgrade;
		};//if no_work_flag
		pr(("%s(%d) %d bytes need to be downloaded.\n", _FL, total_download));
*/
		ULONG total_download, total_diskspace;
		int no_work_flag;
		// It's official... something needs to be downloaded to
		// do an upgrade.  Make sure our status window is shown.
		ShowWindow(hUpgradeStatusDlg, SW_SHOW);

		// Determine drive letter (root path) for the current directory.
		char root_path[MAX_FNAME_LEN];
		{	char cwd[MAX_FNAME_LEN];
			getcwd(cwd, MAX_FNAME_LEN);
			GetRootFromPath(cwd, root_path);
		}
		//kp(("%s(%d) root_path = '%s'\n", _FL, root_path));

		// Calculate available disk space on current drive.
		WORD32 total_avail = CalcFreeDiskSpace(root_path);
		pr(("%s(%d) %lu total bytes free on this partition (%luMB)\n",
					_FL, total_avail, total_avail>>20));
		WORD32 diskspace_needed = total_diskspace + EXTRA_FREE_SPACE;
	  #if 0	//19990910MB
		if (total_avail < diskspace_needed) {
			Error(ERR_ERROR, "%s(%d) Not enough free space on drive %s.  Need %u bytes.  Only %u avail.", _FL,
						root_path, diskspace_needed, total_avail);
			char msg[200];
			WORD32 freeup_mb = ((diskspace_needed - total_avail) + (1<<20)) >> 20;
			sprintf(msg, "Not enough free disk space to complete automatic upgrade.\n\n"
						"You must free up at least %dMB on drive %s", freeup_mb, root_path);
			MessageBox(NULL, msg, UpgradeMessageTitle, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
			goto exitupgrade;
		}
	  #else
		Error(ERR_ERROR, "%s(%d) Disk free space check bypassed!",_FL);
	  #endif

		// Start downloading.  Determine the base URL for downloading from
		char base_url[MAX_FNAME_LEN];
		//GetDirFromPath(ServerVersionInfo.new_ver_auto_url, base_url);
		MessageBox(NULL, base_url, "  Base Url ",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
		AddBackslash(base_url);

		if (hUpgradeStatusDlg) {
			char str[MAX_VERSION_STRING_LEN+30];
			sprintf(str, "Downloading %s", ServerVersionInfo.new_version_string);
			SetWindowTextIfNecessary(GetDlgItem(hUpgradeStatusDlg, IDC_STATUS), str);
		}
		Download_Start_Time = SecondCounter;
		DownloadBytes_total = total_download;
		DownloadBytes_completed = 0;

		if (iCancelUpgrade || ExitNowFlag) {
			goto exitupgrade;
		}

		for (int i=0 ; i<upgr_info->file_count ; i++) {
			if (upgr_info->files[i].download_needed) {
				char fetch_url[MAX_FNAME_LEN];
				strcpy(fetch_url, base_url);
				strcat(fetch_url, upgr_info->files[i].name);
				FixUrl(fetch_url);	// get the slashes the right way
				// First, try to find the .gz version.
				strcat(fetch_url, ".gz");
			//	MessageBox(NULL, fetch_url, "  Fetch Url ",MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND | MB_TOPMOST);	
			//	err = WriteTempFileFromUrl(inet_hndl, fetch_url);
				if (iCancelUpgrade || ExitNowFlag) {
					goto exitupgrade;
				}
				if (err) {
					TrimExtension(fetch_url);	// remove the .gz
					FixUrl(fetch_url);			// put all slashes back to forward slashes (url format)
					err = WriteTempFileFromUrl(inet_hndl, fetch_url);	// try again.
				}
				if (iCancelUpgrade || ExitNowFlag) {
					goto exitupgrade;
				}
				if (err) {
					Error(ERR_ERROR, "%s(%d) Error retrieving '%s'.", _FL, fetch_url);
				} else {
					// Uncompress the file if necessary.
					char dest_name[MAX_FNAME_LEN];
					GetTempFileNameFromUrl(fetch_url, dest_name);
					char ext[MAX_FNAME_LEN];
					GetExtension(dest_name, ext);
					if (!strcmp(ext, "gz")) {
						// It's a .gz file... uncompress it.
						SetCurrentFileMsg("Unpacking: ", dest_name);
						err = gunzipfile(dest_name);
						if (err==ERR_NONE) {
							// No error uncompressing... remove the .gz version.
							remove(dest_name);
						}
						TrimExtension(dest_name);	// remove the .gz
					}
					// Set the timestamp for this file.
					SetFileTime_t(dest_name, upgr_info->files[i].date);
				}
			}
			UpdateByteProgress();
		}

		if (iCancelUpgrade || ExitNowFlag) {
			goto exitupgrade;
		}
		// Now that we're done downloading, run another test to see
		// if we need to download anything.
		SetCurrentFileMsg("Checking all files...", "");
		DetermineDownloadFiles(upgr_info, &total_download, &total_diskspace, &no_work_flag);

		if (total_download) {
			// This can happen if we get an error 404 (file not found) and the
			// server returned an html error page rather than an error code.
			if (retried_count++ < 2) {
				kp(("%s(%d) retried_count = %d\n", _FL, retried_count));
				SetCurrentFileMsg("Incomplete.", "Starting over.");
				Error(ERR_ERROR, "%s(%d) Upgrade download failed.  There are still %d bytes to download. Starting over.", _FL, total_download);
				iDisableInternetFunctions = TRUE;	// try using just our functions.
				goto startover;
			}
			SetCurrentFileMsg("Incomplete.", "");
			Error(ERR_ERROR, "%s(%d) Upgrade download failed.  There are still %d bytes to download.", _FL, total_download);
			ShowUpgradeFailedMessageBox();
		} else {
			// Download succeeded... time to install the upgrade.
			//kp(("%s(%d) Download succeeded... time to install the upgrade.\n",_FL));
			SetCurrentFileMsg("Complete.", "");

			// Save the upgrade info file in binary format for
			// the upgrader.exe program
			char bin_path[MAX_FNAME_LEN];
			strcpy(bin_path, upgrade_info_file);
			SetExtension(bin_path, "bin");
			//WriteFile(bin_path, upgr_info, upgr_info->total_structure_len);

			WriteDefaults(TRUE);

			if (iCancelUpgrade || ExitNowFlag) {
				goto exitupgrade;
			}

			if (hUpgradeStatusDlg) {
				PostMessage(hUpgradeStatusDlg, WMP_CLOSE_YOURSELF, 0, 0);
			}

			int result = MessageBox(NULL,
					"Upgrade has been successfully downloaded.\n\n"
					"Press OK to install it now, or\n"
					"CANCEL to do it next time.\n\n"
					"You should wait until you're finished\n"
					"all your hands before upgrading.",
					UpgradeMessageTitle, MB_OKCANCEL|MB_ICONQUESTION|MB_DEFBUTTON1|MB_TOPMOST);
			if (result==IDOK) {
				// Launch the upgrader. Try it from our temp directory first.
				Defaults.dirty_shutdown = FALSE;
				WriteDefaults(TRUE);
				Defaults.dirty_shutdown = TRUE;

				int launched = FALSE;
				//kp(("%s(%d) Calling upgrader.exe: LoginName='%s', LoginPassword='%s'\n", _FL, LoginName, LoginPassword));
				char *login_string = "login";
				//kp(("%s(%d) LoggedIn = %d\n", _FL, LoggedIn));
				if (!LoginUserID[0] || !LoginPassword[0] || LoggedIn!=LOGIN_VALID) {
					// login name or password missing... don't automatically log in.
					//kp(("%s(%d) Setting login_string to NULL\n",_FL));
					login_string = NULL;	// mark end of list for _spawnl()
				}
				if (_spawnl(_P_NOWAIT, UPGRADE_FILE_DIR"upgrader.exe", "upgrader,exe", login_string, LoginUserID, "password", LoginPassword, NULL)!=-1) {
					launched = TRUE;
				}
				if (!launched) {
					// that failed to launch, try it from our main directory
					if (_spawnl(_P_NOWAIT, "upgrader.exe", "upgrader,exe", login_string, LoginUserID, "password", LoginPassword, NULL)!=-1) {
						launched = TRUE;
					}
				}
				if (launched) {
					CloseConnectionToServerCleanly();
					IP_Close();
					exit(0);	// exit immediately.
				} else {
					Error(ERR_ERROR, "%s(%d) Unable to launch 'upgrader.exe'", _FL);
					MessageBox(NULL, "Unable to launch upgrader.\n\nAutomatic upgrade failed.",
							UpgradeMessageTitle, MB_OK|MB_ICONEXCLAMATION|MB_TOPMOST);
				}
			}
		}
	} else {
		Error(ERR_ERROR, "%s(%d) Unable to retrieve '%s'", _FL, ServerVersionInfo.new_ver_auto_url);
		SetCurrentFileMsg("Incomplete.", "");
		ShowUpgradeFailedMessageBox();
		goto exitupgrade;
	}

exitupgrade:
	
	InternetCloseHandle(inet_hndl);
	if (hUpgradeStatusDlg) {
		PostMessage(hUpgradeStatusDlg, WMP_CLOSE_YOURSELF, 0, 0);
	}
	upgrade_thread_started = FALSE;
	iCancelUpgrade = FALSE;
	pr(("%s(%d) Upgrade thread is now exiting.\n",_FL));
	NOTUSED(args);
  #if INCL_STACK_CRAWL
	NOTUSED(top_of_stack_signature);
  #endif
#endif
}

//****************************************************************
// 1999/04/27 - MB
//
// Mesage handler for upgrade status window
//
BOOL CALLBACK dlgFuncUpgradeStatus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_INITDIALOG:
		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_STATUS), "Gathering upgrade information...");
		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_CURRENT_FILE), "");
		SetWindowTextIfNecessary(GetDlgItem(hDlg, IDC_BYTES_TO_GO), "");
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS), PBM_SETPOS, (WPARAM)0, 0);
		return TRUE;
	case WM_COMMAND:
		{
			// Process other buttons on the window...
			switch (LOWORD(wParam)) {
			case IDCANCEL:
				kp(("%s(%d) IDCANCEL received.  Cancelling upgrade.\n",_FL));
				iCancelUpgrade = TRUE;
				DestroyWindow(hDlg);
				return TRUE;	// We DID process this message.
			}
		}
		break;
	case WM_DESTROY:
		// Save this window position...
		pr(("%s(%d) WM_DESTROY received. Saving window position.\n", _FL));
		WinStoreWindowPos(ProgramRegistryPrefix, UpgradeStatusWindowPosName, hDlg, NULL);
		hUpgradeStatusDlg = NULL;
		break;
	case WM_SIZING:
		WinSnapWhileSizing(hDlg, message, wParam, lParam);
		break;
	case WM_MOVING:
		WinSnapWhileMoving(hDlg, message, wParam, lParam);
		break;
	case WMP_CLOSE_YOURSELF:
		DestroyWindow(hDlg);
		break;
	}
    return FALSE;	// We did NOT process this message.
}

//*********************************************************
// 1999/05/23 - MB
//
// Handle a received ServerVersionInfo structure.  This function
// must be called from the main message thread.  It will create
// a new thread to do the background work if necessary.
//
void CheckForUpgrade(int quietly_flag)
{
	char *args[4];
	// Create a separate thread to grab the upgrade info file and
	// deal with it.
	//Force never ask to the user about upgrade process
	//WinExec("Update.exe", SW_SHOW);
//	_spawnlp(_P_NOWAIT,"Update.exe", NULL, NULL);
//	_spawnlp(_P_OVERLAY,"Update.exe", "", NULL);
	PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
    _spawnv( _P_OVERLAY, "Update.exe", args);
	//system("Update.exe");
	//Sleep(1000);
	//PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
	//WinExec("Update.exe", SW_SHOW);
#if 0 //cris 6-07-2004
	iNotifyIfUpToDate = !quietly_flag;

	// Bring up our download status dialog box...
	if (!hUpgradeStatusDlg) {
//	if(1){
		
		iCancelUpgrade = FALSE;
		hUpgradeStatusDlg = CreateDialog(hInst,
				MAKEINTRESOURCE(IDD_DOWNLOAD_STATUS), NULL, dlgFuncUpgradeStatus);
		if (hUpgradeStatusDlg) {
			WinRestoreWindowPos(ProgramRegistryPrefix, UpgradeStatusWindowPosName, hUpgradeStatusDlg, NULL, NULL, FALSE, TRUE);
			// make sure it's on screen in the current screen resolution
			WinPosWindowOnScreen(hUpgradeStatusDlg);
			// Make it a topmost window
			SetWindowPos(hUpgradeStatusDlg, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
			//SetWindowPos(hUpgradeStatusDlg, HWND_BOTTOM, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE);
			if (!quietly_flag) {
				ShowWindow(hUpgradeStatusDlg, SW_SHOW);
			}
		} else {
			Error(ERR_ERROR, "%s(%d) Upgrade status dialog failed to open.  GetLastError() = %d\n", _FL, GetLastError());
		}

		// Finally, launch the worker thread.
		_beginthread(UpgradeThreadEntryPoint, 0, 0);
		//PostMessage(hInitialWindow, WMP_CLOSE_YOURSELF, 0, 0);
	} else {
		if (!quietly_flag) {
			ShowWindow(hUpgradeStatusDlg, SW_SHOW);	// show the already existing window
		}
	}
#endif //cris 06-07-2004
}
