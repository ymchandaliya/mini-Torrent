## Overview

This system allows users to share and download files within groups they belong to. Downloads are performed in parallel, with pieces of files retrieved from multiple peers to ensure efficient and fast file transfer. The file is divided into 512KB pieces, and each piece is hashed using SHA1 for integrity verification.

## Features

1. **User Management**:
   - Create an account and register with the tracker.
   - Login using user credentials.
   - Logout and temporarily stop sharing files.

2. **Group Management**:
   - Create a group and become its owner.
   - List all groups available on the server.
   - Request to join a group.
   - Leave a group.
   - Accept or reject group join requests (for group owners).

3. **File Sharing**:
   - Share files within a group by providing the filename and SHA1 hash.
   - List all sharable files within a group.
   - Download files from multiple peers, ensuring file integrity through SHA1 comparison.

Client Commands
 - Create User Account: create_user <user_id> <password>
 - Login: login <user_id> <password>
 - Create Group: create_group <group_id>
 - Join Group: join_group <group_id>
 - Leave Group: leave_group <group_id>
 - List Pending Join: list_requests<group_id>
 - Accept Group Joining Request: accept_request <group_id> <user_id>
 - List All Group In Network: list_groups
 - List All sharable Files In Group: list_files <group_id>
 - Upload File: upload_file <file_path> <group_id>
 - Download File: download_file <group_id> <file_name> <destination_path>
 - Logout: logout

Client:
 - Users must create an account and log in to be part of the network.
 - Users can create groups and become owners of those groups.
 - To download files, users must be part of the group sharing the file.
 - Group join requests are managed by the group owner.
 - Shared files are not uploaded to the tracker; only the client's IP and port are shared.
 - The tracker provides peer information for downloading files.
 - Clients download different pieces of files from multiple peers simultaneously using a custom piece selection algorithm.
 - Downloaded pieces are immediately available for sharing.
 - Shared files stop being available upon logout and resume upon the next login.
 - Trackers remain synchronized with each other.

File Integrity
 - Files are divided into 512KB pieces.
 - Each piece is hashed using SHA1.
 - The combined SHA1 hash of all pieces is used for integrity verification during download.
 - This system ensures efficient and secure file sharing within groups, with robust user and group management capabilities.

## Key Areas
- C++
- Multithreading
- SHA1
- Socket Programming
