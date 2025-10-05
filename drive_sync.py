# drive_sync.py

import os
import io
import time
import hashlib
import concurrent.futures # Added for parallelism
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build
from googleapiclient.errors import HttpError
from googleapiclient.http import MediaIoBaseDownload

# --- SETUP INSTRUCTIONS ---
# 1. Enable the Google Drive API:
#    Go to the Google Cloud Console (https://console.cloud.google.com/).
#    Create a new project.
#    Navigate to "APIs & Services" -> "Enabled APIs & services".
#    Click "Enable APIs and Services" and search for "Google Drive API".
#    Enable it.

# 2. Create credentials:
#    In "APIs & Services", go to "OAuth consent screen".
#    Configure it (e.g., set the app name and scope). For this script,
#    you'll need the 'https://www.googleapis.com/auth/drive.readonly' scope.
#    Then, go to "Credentials" and click "CREATE CREDENTIALS" -> "OAuth client ID".
#    Select "Desktop app" and create the client ID.
#    Download the `client_secret.json` file. Save it in the same directory as this script.

# 3. Install necessary libraries:
#    Run `pip install google-api-python-client google-auth-oauthlib google-auth-httplib2`

# --- CONFIGURATION ---
# The local directory where you want to sync your Google Drive files.
# Make sure this directory exists or the script will create it.
SYNC_DIR = 'D:\\GoogleDrive'

# The path to your downloaded client_secret.json file.
CLIENT_SECRET_FILE = 'client_secret.json'

# The file to store your user credentials after the first authentication.
TOKEN_FILE = 'token.json'

# Google Drive API scopes. 'drive.readonly' is sufficient for a one-way sync.
SCOPES = ['https://www.googleapis.com/auth/drive.readonly']

# The maximum number of files to download/check concurrently.
MAX_WORKERS = 20

# A mapping for Google Workspace file types to their export MIME types.
MIME_EXPORT_MAPPING = {
    'application/vnd.google-apps.document': 'application/pdf',
    'application/vnd.google-apps.spreadsheet': 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
    'application/vnd.google-apps.presentation': 'application/vnd.openxmlformats-officedocument.presentationml.presentation',
    'application/vnd.google-apps.drawing': 'application/pdf',
    'application/vnd.google-apps.form': 'application/pdf',
    'application/vnd.google-apps.script': 'application/vnd.google-apps.script+json',
}


def authenticate_google_drive():
    """
    Authenticates with the Google Drive API.
    Handles the OAuth 2.0 flow to get user credentials.
    """
    creds = None
    # The token.json file stores the user's access and refresh tokens.
    if os.path.exists(TOKEN_FILE):
        creds = Credentials.from_authorized_user_file(TOKEN_FILE, SCOPES)

    # If there are no (valid) credentials available, log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            print("Credentials expired. Attempting to refresh...")
            creds.refresh(Request())
        else:
            print("No valid credentials found. Launching authentication flow...")
            flow = InstalledAppFlow.from_client_secrets_file(
                CLIENT_SECRET_FILE, SCOPES)
            creds = flow.run_local_server(port=0)

        # Save the credentials for the next run.
        with open(TOKEN_FILE, 'w') as token:
            token.write(creds.to_json())
    return creds


def get_drive_service():
    """Builds and returns the Google Drive API service."""
    try:
        creds = authenticate_google_drive()
        service = build('drive', 'v3', credentials=creds)
        return service
    except HttpError as error:
        print(f"An HTTP error occurred: {error}")
        return None
    except FileNotFoundError:
        print(f"Error: '{CLIENT_SECRET_FILE}' not found.")
        print("Please follow the setup instructions to get your credentials file.")
        return None


def download_file(service, file_id, file_path, mime_type):
    """
    Downloads a file from Google Drive to the specified local path.
    Handles both binary files and Google Workspace files.
    """
    file_name = os.path.basename(file_path)
    try:
        # Check if the file is a Google Workspace file.
        if mime_type in MIME_EXPORT_MAPPING:
            export_mime_type = MIME_EXPORT_MAPPING[mime_type]
            request = service.files().export_media(fileId=file_id, mimeType=export_mime_type)
            # Update the file extension based on the export type
            file_extension = '.' + export_mime_type.split('.')[-1]
            file_path = os.path.splitext(file_path)[0] + file_extension
        else:
            request = service.files().get_media(fileId=file_id)

        file_handle = io.BytesIO()
        downloader = MediaIoBaseDownload(file_handle, request)
        done = False
        while not done:
            status, done = downloader.next_chunk()
            # Note: Progress is often difficult to display accurately when running in parallel threads
            # print(f"[{file_name}] Download progress: {int(status.progress() * 100)}%")
        
        file_handle.seek(0)
        
        # Create the directories if they don't exist (should already be done, but a safety check)
        os.makedirs(os.path.dirname(file_path), exist_ok=True)
        
        with open(file_path, 'wb') as f:
            f.write(file_handle.read())
        print(f"[{file_name}] Downloaded to '{file_path}' successfully.")
        return True
    except HttpError as error:
        print(f"[{file_name}] An error occurred while downloading file ID '{file_id}': {error}")
        return False
    except Exception as e:
        print(f"[{file_name}] An unexpected error occurred during download: {e}")
        return False


def calculate_md5(file_path):
    """
    Calculates the MD5 checksum of a local file.
    """
    try:
        hash_md5 = hashlib.md5()
        with open(file_path, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hash_md5.update(chunk)
        return hash_md5.hexdigest()
    except Exception as e:
        print(f"Error calculating MD5 for '{file_path}': {e}")
        return None


def sync_single_file(service, item, local_path):
    """
    The worker function executed by the thread pool to check and sync a single file.
    """
    file_id = item['id']
    file_name = item['name'].strip()
    mime_type = item['mimeType']
    drive_md5 = item.get('md5Checksum')

    # Check if the file already exists locally.
    if os.path.exists(local_path):
        # For Google Workspace files, MD5 checksum is not available, so we'll always download.
        if not drive_md5:
            print(f"[{file_name}] is a Google Docs file. Downloading updated version...")
            download_file(service, file_id, local_path, mime_type)
        else:
            # Compare MD5 checksums for regular files.
            local_md5 = calculate_md5(local_path)

            if drive_md5 and local_md5 and drive_md5 != local_md5:
                print(f"[{file_name}] has a different MD5 checksum. Downloading updated version...")
                download_file(service, file_id, local_path, mime_type)
            else:
                print(f"[{file_name}] is already up to date (MD5 matches). Skipping.")
    else:
        # File does not exist locally, download it.
        print(f"[{file_name}] not found locally. Downloading...")
        download_file(service, file_id, local_path, mime_type)


def list_and_sync_files(service):
    """
    Lists all files in Google Drive, builds paths, creates folders synchronously,
    and syncs files concurrently.
    """
    print(f"Starting sync to '{SYNC_DIR}' using {MAX_WORKERS} parallel threads...")
    
    if not os.path.exists(SYNC_DIR):
        os.makedirs(SYNC_DIR)
        print(f"Created local sync directory: '{SYNC_DIR}'")

    files_to_process = []
    page_token = None

    # PHASE 1: List all files/folders and build a list of tasks (synchronously)
    print("Phase 1: Listing and preparing files/folders...")
    while True:
        try:
            results = service.files().list(
                q="trashed = false", 
                fields="nextPageToken, files(id, name, mimeType, parents, md5Checksum)",
                pageSize=100,
                pageToken=page_token
            ).execute()
        except HttpError as error:
            print(f"An error occurred while listing files: {error}")
            return

        items = results.get('files', [])
        if not items and not page_token:
            print("No files found in Google Drive.")
            break

        for item in items:
            file_name_clean = item['name'].strip()
            mime_type = item['mimeType']
            is_folder = mime_type == 'application/vnd.google-apps.folder'

            # Construct the local path.
            path_parts = [file_name_clean]
            parents = item.get('parents', [])
            while parents:
                parent_id = parents[0]
                try:
                    parent_item = service.files().get(fileId=parent_id, fields="name, parents").execute()
                    # Clean parent directory names as well
                    path_parts.insert(0, parent_item['name'].strip())
                    parents = parent_item.get('parents', [])
                except HttpError:
                    break # Stop if parent not found

            local_path = os.path.join(SYNC_DIR, *path_parts)

            if is_folder:
                # Synchronously create all folders to ensure file paths are valid later
                if not os.path.exists(local_path):
                    os.makedirs(local_path, exist_ok=True)
                    print(f"Created folder '{local_path}'")
            else:
                # Add file to the queue for concurrent processing
                files_to_process.append((item, local_path))

        page_token = results.get('nextPageToken')
        if not page_token:
            break
    
    if not files_to_process:
        print("No files to sync.")
        print("Sync complete.")
        return

    # PHASE 2: Process files concurrently
    print(f"Phase 2: Starting concurrent sync of {len(files_to_process)} files...")
    
    # Using ThreadPoolExecutor for I/O-bound tasks
    with concurrent.futures.ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:
        # Submit all file sync tasks
        future_to_file = {
            executor.submit(sync_single_file, service, item, local_path): item['name'].strip()
            for item, local_path in files_to_process
        }
        
        # Wait for tasks to complete and handle any exceptions
        for future in concurrent.futures.as_completed(future_to_file):
            file_name = future_to_file[future]
            try:
                # Check the result of the future to surface exceptions
                future.result() 
            except Exception as exc:
                print(f"'{file_name}' generated an unexpected exception: {exc}")

    print("Sync complete. (Concurrent phase finished)")


def main():
    service = get_drive_service()
    if service:
        list_and_sync_files(service)


if __name__ == '__main__':
    main()
