/*psNew.c, by Jordan Simons. Reimplementation of the ps command.
Note: I discussed with Asha how to parse the status file prior to completing that section.
	Addtionally: We discussed how to get the arguments from cmdline. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char *argv[])
{
	int errorCheck, Uid, cmdline, cmdLineStringLen;
	FILE *statusFile;
	struct dirent *direntPoint;
	DIR *dirPoint;
	char *ptr, *token1, *token2, *token3 = NULL;
	const int LONGEST_LINE = 100;
	const int maxCommandLine = 4096;
	char PPid[20], State[80], User[80], subDirectory[100], buffer [LONGEST_LINE];
	struct passwd *userPasswd;
	int bytesRead = 1;
	//We assume the max commandline length is 4096
	char *cmdlineBuffer = malloc(maxCommandLine * sizeof(char *));

	//psNew takes no additional commandline arguments.
	if (argc == 1)
	{

		//Open /proc directory, checking for failure.
		dirPoint = opendir("/proc");
		if (dirPoint == NULL)
		{
			perror("opendir /proc");
			return EXIT_FAILURE;
		}

		//Look at every directory in proc
		while ( (direntPoint = readdir(dirPoint)) != NULL )
		{
			char  *currentPID = direntPoint->d_name;
			//We only want directories related to processes, which have numerical names.
			if ( (strtol(currentPID, &ptr , 10))  != 0)
			{
				strcpy(subDirectory, "/proc/");
				strcat(subDirectory, currentPID);
				strcat(subDirectory, "/status");
				//We first want the status file in each subdirectory.

				//Open status file.
			      statusFile = fopen(subDirectory, "r");
			      if (statusFile == NULL) {
			          perror("open statusFile");
			      }

			    //Read from status file line by line
			      while(fgets(buffer, LONGEST_LINE, statusFile) != NULL)
			      {
			      	//create token for PPid
			      	token1 = strstr(buffer, "PPid:" );

			      	//We only set the value if we read the line containing it.
			      	if(token1 != NULL)
			      	{
			      		sscanf(token1, "PPid:%s", PPid);
			      		continue;
			      	}

			      	//create token for State
			      	token2 = strstr(buffer, "State:");

			      	if (token2 != NULL)
			      	{
			      		sscanf(token2, "State:%s", State);
			      		continue;
			      	}

			      	//create token for Uid
			      	token3 = strstr(buffer, "Uid:");

			      	if (token3 != NULL)
			      	{
			      		sscanf(token3, "Uid:%d", &Uid);
			      		continue;
			      	}

			      	//We use getpwuid() to get the user's name from a corresponding Uid
			      	userPasswd = getpwuid(Uid);

			      	strcpy(User, userPasswd->pw_name);

			      	/*Note: the above strcpy() caused a segfault occasionally.
			      		I believe it's completely fixed, but the below check remedied it.

			      	if (userPasswd != NULL)
			      	{
			      		strcpy(User, userPasswd->pw_name);

			      	}

			      	else {
			      		strcpy(User, "ERROR");
			      	} */

			      	/*We know that by the end of the loop each variable will contain a value,
			      	 as each status file has this setup. */

			      }

			      //Close status
			      errorCheck = fclose(statusFile);
			      if (errorCheck == -1)
			      {
			      	perror("Close status");
			      }

			      //Print contents gathered from Status
			      printf("%s \t %s \t %s \t %s \t", currentPID, PPid, State, User);

			     //Set subDirectory to the cmdline file.
			    strcpy(subDirectory, "/proc/");
				strcat(subDirectory, currentPID);
				strcat(subDirectory, "/cmdline");

			    //Open cmdline file.
			    cmdline = open(subDirectory, O_RDONLY);
			    if (cmdline == -1) {
			        perror("open cmdline");
			    }

        		//Reads entire cmdline file contents.
			    bytesRead = read(cmdline, cmdlineBuffer, maxCommandLine);

			    //Reports error
			    if (bytesRead == -1)
			        {
			          perror(subDirectory);
			          return EXIT_FAILURE;
			        }

			    //Ensures we print nothing if there is no content in cmdline.
			    else if (bytesRead == 0)
			    {
			    	printf("%s \n", "");
			    }

			    //At this point, cmdlineBuffer contains the command name.
			    else
			    {

			    	//Find length of the string up to the first terminating NULL.
			    	cmdLineStringLen = strlen(cmdlineBuffer);

			   		//Print command and any arguments.
			    	while (cmdLineStringLen)
			    	{

			    		printf("%s \t", cmdlineBuffer);
			    		cmdlineBuffer += cmdLineStringLen + 1;
			    		cmdLineStringLen = strlen(cmdlineBuffer);

			    	}
			    	printf("\n");
				}

				 //Close cmdline
			      errorCheck = close(cmdline);
			      if (errorCheck == -1)
			      {
			      	perror("Close cmdline");
			      }

			}

		}

		//Close directory, checking for failure.
		errorCheck = closedir(dirPoint);
		if (errorCheck == -1)
		{
			perror("closedir dirPoint");
		}

		return EXIT_SUCCESS;
	}

	else
	{
		printf("No additional command line paramaters. \n");
		return EXIT_FAILURE;
	}
}
