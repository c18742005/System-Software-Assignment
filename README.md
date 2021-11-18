# Rich Web App Tech - Lab 1
## Lab Instructions
A manufacturingcompany has been experiencing difficulties getting current information from specific departments to see the current state of the plant. At certain points in the past manufacturing had to stop due to lack of raw materials andlimited space in the loading bay. The managers from the following departments need to complete a report on a daily basis to feed into a dashboard reporting system:
    - Warehouse
    - Manufacturing
    - Sales
    - Distribution

The department managers have to upload the xml report to a shared directory for collection by the dashboard system. Problems have occurred in the past where changes were made to the shared directory incorrectly and it wasn’t possible to track who made the changes. The company’s CTO has listed the functionality they would like to include in their new business model to offer transparency and accountability for all changes made to the shared directory.

### How the website management currently works:
- The above departments are responsible for reporting on a daily basis. 
- Reports need to be submitted for inclusion in the daily dashboard report. 
- Debian Managershave an account on a Debian server, they can login and make changes to the shared directory. All changes will be made under their user accounts.
- There have been disputes in the past over who is responsible for filing reports and for reports allegedly disappearing from the shared directory.

### What the CTO wants:
The CTO has offered a list of desired functionality for the new website management model:
1. The company will offer a new shared directory for the 4 departments to upload their xml reports. Authorised users can make changes to directory. The reports will be collected on a nightly basis at 1am and moved to the directory for the dashboard system for processing.
2. The dashboard directory content should be backedup every night.
3. The changes made to the department managers upload directory needs to documented. The username of the user, the file they modified and the timestamp should be recorded.
4. No changes should be allowed to be made to the directories (upload and reporting) while the backup/transfer is happening.
5. If a change needs to be urgently made to the live site, it should be possible to make the changes.

### Project Requirements
- Create a daemon to continually manage the operation of the requirements listed by the CTO above.
- Identify new or modified xml reports and log details of who made the changes, this should be generated as a text file report and stored on the server.
- The department managers must upload their xml report file by 11.30pm each night. The uploads to the shared directory must be moved to the reporting directory. This should happen at 1am. (this functionality needs to be self contained in the solution. Don’t use cron)
- If a file wasn’t uploaded this should be logged in the system. (A naming convention can be used to help with this task.)
- When the backup/transfer begins no user should be able to modify either the upload or reporting directory.
- It must be possible to ask the daemon to backup and transfer at any time.
- IPC should be setup to allow all processes to report in on completion of a task. (success or failure)
- Error logging and reporting should be included for all main tasks in the solution.
- Create a makefile to manage the creation of the executables

### General Assumptions
- The company only hasone server
- Backups can be made to a different directoryon the server
- The upload and reporting directories should be two sub directories (you can choose this location in your system)

## How to run the project
- Open the src/ directory in a terminal 
- Run 'make' to compile the program 
- Execute the program by using the command './manufacturingDaemon'
- More instructions can be found in the document included in the main directory