// Kelsey Dowd
// Operating Systems PA1
// Feb 2 2016

#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main() {
	int result;
	syscall(324, 10, 7, &result);
	printf("Sum of Numbers: %d", result);
	return 0;
}
