#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define SIZE 1024

int main() {
	char buffer[SIZE];
	int device = open("/dev/simple_char_driver", O_RDWR);
	char command = 'n';
	printf("Please pick one of the following:\n r to read from the device\n w to write to the device\n e to exit\n anything to continue reading\n Enter Command: ");
	while(1) {
		scanf("%c", &command);
		
		if (command == 'e') {
			break;
		}
		
		if (command == 'r') {
			read(device, buffer, SIZE);
			printf("%s\n\n", buffer);
			printf("Please pick one of the following:\n r to read from the device\n w to write to the device\n e to exit\n anything to continue reading\n Enter Command: ");

		}
		else if(command == 'w') {
			printf("Enter what you want to write: ");
			scanf("%s", buffer);
			write(device, buffer, strlen(buffer));
			printf("Finished Writing\n\n");
			printf("Please pick one of the following:\n r to read from the device\n w to write to the device\n e to exit\n anything to continue reading\n Enter Command: ");

		}
		else if(command == '\n') {
			
		}
		else {
			printf("\nPlease pick one of the following:\n r to read from the device\n w to write to the device\n e to exit\n anything to continue reading\n Enter Command: ");
		}
	}
	
	
	return 0;
}
