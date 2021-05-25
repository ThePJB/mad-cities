#include <string.h>

//Convert an integer to a roman number format
//The return value returns if the input was successfully converted or not
int int2roman(int input, char * output){
	//Check to see if int is with in range
	if (input < 0 || input > 4000){
		return 0;
	}
	
	strcpy(output,"");
	
	//Some data structions to help us do the conversion
	int num_possibilities = 13;
	const char *options[num_possibilities];
	options[0] = "M";
	options[1] = "CM";
	options[2] = "D";
	options[3] = "CD";
	options[4] = "C";
	options[5] = "XC";
	options[6] = "L";
	options[7] = "XL";
	options[8] = "X";
	options[9] = "IX";
	options[10] = "V";
	options[11] = "IV";
	options[12] = "I";
	int boundaries[] = {1000,900,500,400,100,90,50,40,10,9,5,4,1};
	
	//Convert the string
	while (input != 0){
		int boundary = 0;
		
		int i = 0;
		for (i = 0; i < num_possibilities; i++){
			if (input >= boundaries[i]){
				boundary = i;
				break;
			}
		}	
		input = input - boundaries[boundary];	
		strcat(output,options[boundary]);
	}
	
	//Return status
	return 1;
}