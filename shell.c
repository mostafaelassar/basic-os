void parse(char*);
void clear(char*);

int main(){
	while(1){
		char input[80];
		interrupt(0x21,0,"SHELL> ",0,0);
		interrupt(0x21,1,input,0,0);
		parse(input);
		clear(input);
	}
	return 0;
}

void parse(char* input){
	int i,j;
	int x,length;
	char view[13312];
	char filename[6];
	char filename2[6];
	char dir[512];
	char write[13312];
	char read[400];
	clear(read);
	clear(write);
	clear(dir);
	clear(view);
	clear(filename);
	clear(filename2);
	if(input[0]=='v'&&input[1]=='i'&&input[2]=='e'&&input[3]=='w'&&input[4]==' '){
		for(i=0;input[i+5]!=0xd&&i<6;i++){
			filename[i]=input[i+5];
		}
		if(input[i+5]=='\r'&&input[i+6]=='\n'&&input[i+7]=='\0'){
			interrupt(0x21, 3, filename, view, 0);
			if(view[0]!='\0'){
				interrupt(0x21, 0, view, 0, 0);
			}
			else{
				interrupt(0x21,0,"File does not exist!\r\n", 0, 0);
			}
			return;
		}
	}
	if(input[0]=='e'&&input[1]=='x'&&input[2]=='e'&&input[3]=='c'&&input[4]=='u'
	&&input[5]=='t'&&input[6]=='e'&&input[7]==' '){
		for(i=0;input[i+8]!=0xd&&i<6;i++){
			filename[i]=input[i+8];
		}
		if(input[i+8]=='\r'&&input[i+9]=='\n'&&input[i+10]=='\0'){
			interrupt(0x21,3, filename, view, 0);
			if(view[0]!='\0'){
				interrupt(0x21, 4,filename, 0x2000, 0);
			}
			else{
				interrupt(0x21,0,"File does not exist!\r\n", 0, 0);
			}
			return;
		}
	}
	if(input[0]=='d'&&input[1]=='e'&&input[2]=='l'&&input[3]=='e'&&input[4]=='t'
	&&input[5]=='e'&&input[6]==' '){
		for(i=0;input[i+7]!=0xd&&i<6;i++){
			filename[i]=input[i+7];
		}
		if(input[i+7]=='\r'&&input[i+8]=='\n'&&input[i+9]=='\0'){
			interrupt(0x21,3, filename, view, 0);
			if(view[0]!='\0'){
				interrupt(0x21, 7,filename, 0, 0);
			}
			else{
				interrupt(0x21,0,"File does not exist!\r\n", 0, 0);
			}
			return;
		}
	}

	if(input[0]=='c'&&input[1]=='o'&&input[2]=='p'&&input[3]=='y'&&input[4]==' '){
		for(i=0;input[i+5]!=' '&&i<6;i++){
			filename[i]=input[i+5];
		}
		if(input[i+5]==' '){
			i++;
			for(j=0;input[i+5]!=0xd&&j<6;j++){
				filename2[j]=input[i+5];
				i++;
			}
			if(input[i+5]=='\r'&&input[i+6]=='\n'&&input[i+7]=='\0'){
				interrupt(0x21,3, filename, view, 0);
				if(view[0]!='\0'){
					length=0;
					for(x=0;view[x]!='\0'&&view[x+1]!='\0';x++){
						length++;
					}
					interrupt(0x21,8,filename2, view, (intDiv(length,512)+1));
				}
				else{
					interrupt(0x21,0,"File does not exist!\r\n", 0, 0);
				}
				return;
			}
		}
	}

	if(input[0]=='d'&&input[1]=='i'&&input[2]=='r'&&input[3]==0xd){
		interrupt(0x21, 9, dir, 0, 0);
		interrupt(0x21, 0, dir, 0, 0);
		return;
	}
	
	if(input[0]=='c'&&input[1]=='r'&&input[2]=='e'&&input[3]=='a'&&input[4]=='t'
		&&input[5]=='e'&&input[6]==' '){
		for(i=0;input[i+7]!=0xd&&i<6;i++){
			filename[i]=input[i+7];
		}
		if(input[i+7]=='\r'&&input[i+8]=='\n'&&input[i+9]=='\0'){
			j=0;
			while(1){
				interrupt(0x21,1,read,0,0);
				if(read[0]==0xd){
					break;
				}
				for(i=0;read[i]!='\0';i++){
					write[j]=read[i];
					j++;
				}
				clear(read);		
			}
			length=0;
			for(x=0;write[x]!='\0'&&write[x+1]!='\0';x++){
				length++;
			}
			interrupt(0x21,8,filename, write, (intDiv(length,512)+1));
			return;
		}
	}

	interrupt(0x21,0,"Bad Command\r\n", 0, 0);
}

void clear(char* string){
	int i;
	for(i=0;string[i]!='\0';i++){
		string[i]='\0';
	}
}

int intDiv(int x,int y){
	int result=0;
	while(x>=y){
		x=x-y;
		result++;
	}
	return result;
}
