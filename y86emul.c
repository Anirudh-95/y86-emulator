#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int ZF, OF, SF; //Declares zero flag, overflow flag, and sign flag
int registers[9]; //Declares register array
char *memory; //Declares memory array
int program_Counter; //Declares program counter

/*enum function to store the states of the program: different errors that may arise in the program*/
typedef enum{
	AOK, // No error in the program
	HLT, // Halt instruction where the program ends normally 
	INS, // Instruction error has occured 
	ADR,  // Address error has occured 
}state;
	state status;

/*If no operations is to be performed, the program counter is incremented by 1*/
void no_operation(unsigned char *byte){
	program_Counter=program_Counter+ 1;
}


/*Function where the transfers with the registers occur */
void register_transfer(unsigned char *byte){
	unsigned char higher = (memory[program_Counter+1] & 0x0f); // gets the upper and lower value 
    unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;
	int *value = (int *)(memory+program_Counter+2);	
	int *memArray = (int *)(memory+registers[higher]+*value);
	
	switch(*byte){ // chooses case base on required the register transfer
	  case 0x20: //register to register
		registers[higher] = registers[lower];
		program_Counter= program_Counter+2;
		break;

	  case 0x30: //immidiate to register 
		registers[higher]=*value;
		program_Counter=program_Counter+6;
		break;

	  case 0x40: //register to memory
		*memArray=registers[lower];
		program_Counter=program_Counter+6;
		break;

      case 0x50: //memory to register 
		registers[lower] = *memArray;
		program_Counter=program_Counter+6;
		break;

	  default: //default case if none of the other cases work
	    fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    status=INS;
	    break;
    }	
}

/*Function where all the operations such as "add", "subtract", "and", "xor" and multiply are performed*/
void operations(unsigned char *byte){
	//struct_bit *B = (struct_bit *)(memory+program_Counter+1); // storing the memory array in the struct to obtain the upper and lower  Hex values
	unsigned char higher = (memory[program_Counter+1] & 0x0f);
    unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;
	int upperValue= registers[higher];
	int lowerValue= registers[lower];
	ZF=OF=SF=0; //setting all the flags to false
	switch(*byte){
		case 0x60:  //add
			if ((higher >= 0 && higher <= 8) && (lower >= 0 && lower <= 8)){ //check if it is a valid register 
				int sum=upperValue+lowerValue;
				if(sum==0) // zero flag check
					ZF=1;

			    if ((lowerValue>0 && upperValue>0 && sum<0) || (lowerValue<0 && upperValue< 0 && sum>0)) //overflow flag check
					OF = 1;

			    if(sum<0)//sign flag check
					SF=1;

			registers[higher]=sum;
		    program_Counter=program_Counter+2;
		   }
		    else{ // if false change status to Error and program abruptly ends 
		    	fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
		    }
			break;

		case 0x61:  //subtract
			ZF=OF=SF=0;
			if ((higher >= 0 && higher <= 8) && (lower >= 0 && lower <= 8)){
				int subtract=registers[higher]-registers[lower];
				registers[higher]=subtract;
				if(subtract==0) // zero flag check
					ZF=1;

				if ((lowerValue<0 && upperValue>0 && subtract<0) || (lowerValue>0 && upperValue< 0 && subtract>0)) //overflow flag check
					OF = 1;

				if(subtract<0)//sign flag check
					SF=1;

			 registers[higher]=subtract;	
		     program_Counter=program_Counter+2;
		     }
		     else{
		     	fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    	     status=ADR;
		     }
		     break;

		case 0x62:  //and 
			ZF=OF=SF=0;
			if ((higher >= 0 && higher <= 8) && (lower >= 0 && lower <= 8)){
			int and=upperValue & lowerValue;
				if(and==0) // zero flag check
					ZF=1;

				if(and<0)//sign flag check
					SF=1;

		    program_Counter=program_Counter+2;
		}
		else{
			fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    	status=ADR;
		}
		break;

		case 0x63:  //xor 
			ZF=OF=SF=0;
			if ((higher >= 0 && higher <= 8) && (lower >= 0 && lower <= 8)){
			int xor=upperValue ^ lowerValue;
				if(xor==0) // zero flag check
					ZF=1;

				if(xor<0)//sign flag check
					SF=1;

		     program_Counter=program_Counter+2;
		}
		else{
			fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    	status=ADR;
		}
		break;

		case 0x64:  //multiply
		ZF=OF=SF=0;
			if ((higher >= 0 && higher <= 8) && (lower >= 0 && lower <= 8)){
				int multiply=upperValue * lowerValue;
				if(multiply==0) // zero flag check
					ZF=1;

				if ((lowerValue>0 && upperValue>0 && multiply<0) || (lowerValue<0 && upperValue< 0 && multiply>0)) //overflow flag check
					OF = 1;

				if(multiply<0)//sign flag check
					SF=1;

			 registers[higher]=multiply;
		     program_Counter=program_Counter+2;
		     }
		     else{
		     	fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    		status=INS;
		     }
		     break;
		case 0x65:  //compare
			ZF=SF=0;
			if ((higher >= 0 && higher <= 8) && (lower >= 0 && lower <= 8)){
				int cmp=registers[higher]-registers[lower];
				
				if(cmp==0) // zero flag check
					ZF=1;

				if(cmp<0)//sign flag check
					SF=1;
			
		     program_Counter=program_Counter+2;
		     }
		     else{
		     	fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    	     status=ADR;
		     }
		     break;

		 default:
		 	 fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    	 status=INS;
		     break;
	}
}

/*Funtion to convert to string into an integer */
long int stringconvert(char *s){
	long int convert = 0; 
	int i;
    for (i = 0; s[i] != '\0';i++){
        convert = convert*10 + s[i] - '0';
		}  

    return convert; //returning the converted integer value
}

/*Function to perform movsbl*/
void movsbl(){
	unsigned char higher = (memory[program_Counter+1] & 0x0f); //getting the 2 registers 
    unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;
	program_Counter++;
	
	char b1[2],b2[2],b3[2],b4[2]; // creating two bits to store the hex bits 2 at a time 
	//conveting the remaining 8 bits to hex forma and storing it in pairs 
	sprintf(b4, "%02x",memory[program_Counter++]);
	sprintf(b3, "%02x",memory[program_Counter++]);
	sprintf(b2, "%02x",memory[program_Counter++]);
	sprintf(b1, "%02x",memory[program_Counter++]);
	
	char dstring[9];
	dstring[9]='\0';
	snprintf(dstring,sizeof(dstring),"%s%s%s%s ",b1,b2,b3,b4); //convertint the little endian to big endian and storing in a stirng 
	int val = stringconvert(dstring); //converting the string into  a single hex integer and storing it in val
	
	int sByte = (int)memory[registers[higher]+val]; //to obtain the byte that is to be shifted
  	int lByte = sByte & 0x000000ff; // getting the leading value of the byte 
  	int shiftedValue; //to store the value after shifting has occured 
  	
  	//performing sign extension based on the if the byte is positive or negative 
  	if(lByte < 0){
    	shiftedValue = lByte ^ 0xffffff00;
  	} 
  	else {
    	shiftedValue = lByte ^ 0x00000000;
    }
    registers[lower] = shiftedValue; // stroing the value in Register A i.e RA
}

/* Does the various jump statements depending on the flags*/ 
void jump(unsigned char *byte){
	int *value = (int*)(memory+program_Counter+1);
	
	switch(*byte){
		case 0x70:
		program_Counter = *value;
		break;

	case 0x71:
		if (((SF^OF)|ZF) == 1) {
			program_Counter = *value;
		}
		else{
			program_Counter=program_Counter+5;
		}
		break;

	case 0x72:
		if ((SF^OF) == 1) {
			program_Counter = *value;
		}
		else{
			program_Counter=program_Counter+5;
		}
		break;

	case 0x73:
		if (ZF==1) {
			program_Counter = *value;
		}
		else{
			program_Counter=program_Counter+5;
		}
		break;

	case 0x74:
		if (ZF==0){
			program_Counter = *value;
		}
		else{
			program_Counter=program_Counter+5;
		}
		break;

	case 0x75:
		if ((SF^OF)==0) {
			program_Counter = *value;
		}
		else{
			program_Counter=program_Counter+5;
		}
		break;

	case 0x76:
		if (((SF^OF)&ZF)==0){
			program_Counter = *value;
		}
		else{
			program_Counter=program_Counter+5;
		}
		break;

	default:
		fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    status=INS;
		break;
	}
}

void push_function(int index){
		int value = registers[index]; //storing the value of the index 
		int esp_Decrement  = registers[4]-4; //decrementing the esp register 
		int *m = (int *)(memory+esp_Decrement); //obtaing the corresponding the value from the memory block
		*m= value;
		registers[4] = esp_Decrement; //updating the esp register value 
		program_Counter=program_Counter+ 2;
}

void call_function(){
	int *value = (int *)(memory+program_Counter+1); // obtaining the value from the memory block 
	registers[8] = program_Counter+5;
	push_function(8); //sending the 8th index to push a value into the stack 
	program_Counter=*value; //updating the program counter 
}

/*The pop function where a value is poped from the stack */
void pop_function(int index){
		int esp_Value = registers[4]; //storing the value of the esp register 
		int esp_Increment = esp_Value+4; //incrementing the esp register 
		int *m = (int *)(memory + esp_Value); //obtaining the correesponding value from the memory block 
		registers[4] = esp_Increment; // storing the incremented value in the esp register 
		registers[index]= *m;//updating the reegister value
		program_Counter=program_Counter+2;
}

/*This function is for returning a value */
void return_function(unsigned char* byte){
	pop_function(8);//sending the last register to the pop funtion for the value to be returned 
	program_Counter=registers[8];
}

/*function where the read operations are performed*/
void read_Function(unsigned char *byte){
	unsigned char higher = (memory[program_Counter+1] & 0x0f);
    unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;
    int *value = (int *)(memory+program_Counter+2);
    unsigned char char_ent; //variables to store the entered values
    int num_ent;
   
   switch (*byte) { //to choose readL or readB
		//case to read a byte value entered by the user 
		case 0xc0:
			if ((higher == 0xf) && (lower >= 0 && lower <= 8)){
				OF=ZF=SF=0;

				if (scanf("%c", &char_ent) == EOF){ //reading and storing the value entered and checking if its the end of the file 
					ZF = 1;
					memory[higher+*value]=char_ent;	
				}
				else{
					unsigned char *readVal = (unsigned char *)(memory+registers[lower]+*value);
					*readVal = char_ent;
					memory[higher+*value]=char_ent;							
				}

			program_Counter=program_Counter+ 6;
			}
			else{
				fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
			}
		break;

		//case to read a long value entered by the user 
		case 0xc1:
			if ((higher== 0xf) && (lower >= 0 && lower <= 8)){
				OF=SF=ZF= 0;
				
				if (scanf("%d", &num_ent)==EOF) //reading and storing the number entered by the user and checing if its the end of the file 
					ZF = 1;
					
				else{
					int *readVal = (int *)(memory+registers[lower]+*value);
					*readVal=num_ent;	
				}
				program_Counter += 6;
		    }
		    else{
				fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
			}
		break;

	default:
		fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    	status=INS;
		break;
	}
}

/*operation where the write operations are performed */
void write_Function(unsigned char *byte) {
	unsigned char higher = (memory[program_Counter+1] & 0x0f);
    unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;
    int *value = (int *)(memory+program_Counter+2);
	
	switch (*byte) { //to choose writel or writeb
		//case to write a byte character
		case 0xd0:
			if ((higher == 0xf) && (lower >= 0 && lower <= 8)){
				unsigned char *byte_var = (unsigned char *)(memory+registers[lower]+*value); //gettint the value from the memory 
				printf("%c", *byte_var);//the charachter at that instance gets prited 
				program_Counter=program_Counter + 6;
			}
			else{
				fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
			}
		break;

		//to write a long variable
		case 0xd1:
			if ((higher == 0xf) && (lower >= 0 && lower <= 8)) {
				int *long_var = (int *)(memory+registers[lower]+*value);
				printf("%d", *long_var);
				program_Counter=program_Counter + 6;
			}

			else{
				fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
			}
		break;

		default:
		fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    status=INS; 
		break;
	}
}

/*This function decodes the instructions and sends it to the required method*/
int decode_intructions(unsigned char* instruction){
	switch (*instruction){
		case 0x00:
			no_operation(instruction);
			break;

		case 0x10:
			status = HLT;
			break;

		case 0x20:
		case 0x30:
		case 0x40:
		case 0x50:
			register_transfer(instruction);
			break;

		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
			operations(instruction);
			break;
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x76:
			jump(instruction);
			break;

		case 0x80:
			call_function(instruction);
			break;

		case 0x90:
			return_function(instruction);
			break;
		
		case 0xa0:{
			unsigned char higher = (memory[program_Counter+1] & 0x0f);
    		unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;

    		if ((higher== 0xf) && (lower >= 0 && lower <= 8)) {
				push_function(lower);
			}
			else{
				fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
			}
		    break;
		}
		case 0xb0:{
			unsigned char higher = (memory[program_Counter+1] & 0x0f);
    		unsigned char lower = (memory[program_Counter+1] & 0xf0)>>4;

    		if ((higher== 0xf) && (lower >= 0 && lower <= 8)) {
				pop_function(lower);
			}
			else{
				fprintf(stderr,"ERROR: INCORRECT ADDRESS OF REGISTER\n");
	    		status=ADR;
			}
		    break;
		}
		case 0xc0:
		case 0xc1:
		    read_Function(instruction);
		    break;

		case 0xd0:
		case 0xd1:
			write_Function(instruction);
			break;

		case 0xe0:
			movsbl();
			break;

		default:
			fprintf(stderr,"ERROR: INCORRECT INSTRUCTION\n");
	    	status=INS;
			break;
	}
	return 0;
}

/*This function breaks up the array containg the memory and sends it to another method to be decoded*/
void get_intructions(){
	while (status == AOK){ 
		unsigned char *instruction;	
		instruction = (unsigned char*)(memory+program_Counter);
		decode_intructions(instruction);		
	}

	if(status == HLT){
		printf("The program has come to an end.\n");
		exit(0);
	}
	if(status==ADR || status == INS){ 
		exit(1);
	}
}

/*Function where all the directives in a file are seperated */
int seperate_file(FILE *f){
	int line_no=0;// Intializes the line number to 0 which is the first line
	char line[2000]; //Initializes the array containing the lines
	char str[6]; //Initializes the array containging the string
	int address; //Declares address: variable to store the hex value of the address 
	int size; //Declares size: variable to store the size of the file 
	char *token; //Declares token: variable to store the token in each line
	int value;// Declares value: variable to duplicate value of address

	while (fgets(line, 2000, f) != NULL) { //As long as the file is not empty 
		if (line_no== 0){ 
			sscanf(line, "%s %x\n", str, &size); // assigns value to the size variable which is the size of the file
			memory = (char *) malloc(sizeof(char)*size); // allocating space in memory for the file
	    }

		else{ //If the line is not the first line in the file
			token = strtok(line, " \t\n\f\v\r"); // gets the directive of each line 
			
			// this block statement is if the directive is "text"
			if (strcmp(token, ".text") == 0) { 
				token = strtok(NULL, " \t\n\f\v\r"); // gets the next token in the line 
				sscanf(token, "%x", &address); // assigns the a hex value to the address variable 
				program_Counter = address;
				token = strtok(NULL, " \t\n\f\v\r"); 
				value = address;
				int j=0;
				while(j<strlen(token)){
					char c[2];
					memcpy(c, &token[j],2);
					memory[value]=strtol(c,NULL,16);
					value++;
					j=j+2;
				}
			}
		   // this block statement is if the directive is "byte"
			else if (strcmp(token, ".byte") == 0){ 
				token = strtok(NULL, " \t\n\f\v\r"); 	
				sscanf(token, "%x", &address); 
				token = strtok(NULL, " \t\n\f\v\r"); 
				sscanf(token,"%2x",&value);
				memory[address]=(char)value;
			}

			//this block statement is if the directive is "long"
			else if (strcmp(token, ".long") == 0){ 
				token = strtok(NULL, " \t\n\f\v\r"); 
				sscanf(token, "%x", &address); 
				token = strtok(NULL, " \t\n\f\v\r"); 
				sscanf(token, "%d", &value); /*Takes the value of a single char in the token, and stores it in "num"*/
				memory[address] = value;
			}

			// this block statement is if the directive is "string"
			else if (strcmp(token, ".string") == 0){ 
				int j=1;
				//char *nextToken;
				token = strtok(NULL, " \t\n"); 
				sscanf(token, "%x", &address); 
				value=address; 
				token = strtok(NULL, "\n\t"); 
				char s[strlen(token)];
				strcpy(s,token);
				while(j<strlen(token)-1){
					//for(j=0;j<strlen(token); j++){
						memory[value]=(unsigned char)s[j];
						value++;
					//}
					//nextToken = strtok(NULL, " \t\n\f\v\r"); 
					//memory[value] = ' ';
					j++;
					//token=nextToken;
				}
			}
			else if(strcmp(token, ".bss") == 0){ //check if its a .bss directive
             	token = strtok(NULL, "\n\t");
	     		sscanf(token, "%x", &address);
	     		token=strtok(NULL,"\n\t");
	     		sscanf(token, "%d", &value);
	     		for (; value > 0; value--) {
	       			memory[address] = (unsigned char)0; 
	       			address++;
	     		}
	  	    }
		    else { // If an invalid directive is entered
				fprintf(stderr, "ERROR: Invalid directive\n");
				return 0;
			}	
	    }
	    value=0;
	    line_no++;
	}
	fclose(f);
	status=AOK;
	get_intructions();	   
	return 0;
}

int main(int argc, char ** argv){
	if (argc!=2) { /*If there are not 2 arguments*/
		fprintf(stderr, "Error: Invalid Input \n");
		return 1;
	}
	else{
		if (strcmp(argv[1], "-h") == 0) {
			printf("Usage: y86emul <y86 input file>\n");
			return 0;
		}

		else{
			FILE *f=fopen(argv[1], "r"); /*Opens the input, which is a file, for reading*/

			if (f==NULL) { /*If there is no file*/
				fprintf(stderr, "ERROR: The file does not exist\n");				
				return 1;
			}
			else{
				seperate_file(f); 
			}
		}
	}
	return 0;
}