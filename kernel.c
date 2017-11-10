void printString(char*);
void readString(char*);
void readSector(char*,int);
void readFile(char*,char*);
void executeProgram(char*,int);
void terminate();
void writeSector(char*,int);
void deleteFile(char*);
void writeFile(char*,char*,int);
void getDirectory(char*);
void handleInterrupt21(int,int,int,int);

int main(){
	makeInterrupt21();
	interrupt(0x21,4, "shell\0", 0x2000, 0);
	while(1);
	return 0;
}

void printString(char* chars){
	int i=0;
	while(chars[i]!='\0'){
		interrupt(0x10, 0xE*256+chars[i], 0, 0, 0);
		i++;
	}
}

void readString(char* chars){
	int i=0;
	while(1){
		char in=interrupt(0x16, 0, 0, 0, 0);
		interrupt(0x10, 0xE*256+in, 0, 0, 0);
		if(in!=0x8){
			chars[i]=in;
			if(in==0xd){
				chars[i+1]=0xa;
				chars[i+2]=0x0;
				interrupt(0x10, 0xE*256+0xa, 0, 0, 0);
				return;
			}
			i++;
		}
		else{
			if(i>0){
				chars[i-1]=0x0;
				i--;
			}
			else{
				i=0;
			}
			interrupt(0x10, 0xE*256+' ', 0, 0, 0);
			interrupt(0x10, 0xE*256+0x8, 0, 0, 0);
		}
	}
}

int mod(int x,int y){
	while(x>=y){
		x=x-y;
	}
	return x;
}

int intDiv(int x,int y){
	int result=0;
	while(x>=y){
		x=x-y;
		result++;
	}
	return result;
}

void readSector(char* buffer, int sector){
	int CH=intDiv(sector,36);
	int CL=mod(sector,18)+1;
	int DH=intDiv(sector,18);
	int DL=0;
	int CX=CH*256+CL;
	int DX=DH*256+DL;
	DH=mod(DH,2);
	interrupt(0x13, 2*256+1, buffer, CX, DX);
}

void readFile(char* fileName,char* buffer){ 
	int entryNo,fileSecsStart,i,j,incMyAddress;
	int sectors[27];
	char dir[512];
  	readSector(dir, 2);  
	entryNo = exists(dir,fileName);
	if (entryNo==-1){
		printString("This file does not exist!\r\n");
		return;
	}
	else{
		fileSecsStart = entryNo*32+6;
		for (i=0;i<26;i++){
			sectors[i] = dir[fileSecsStart+i];
		}
		sectors[26] = 0;
		j = 0;
		incMyAddress = 0;
		while(sectors[j]!=0x0){
			readSector(buffer+incMyAddress,sectors[j]);
			incMyAddress += 512;
			j++;
		}
	}	
}

int exists(char* dir,char* fileName){
	int i,j,k,flag;
	flag = -1;
	j=0;
	for(i=0;i<16;i++){
		k=0;
		while(k<6){
			if(dir[j+k]==fileName[k]){
				flag=1;
			}
			else{
				flag=-1;
				break;
			}
		k++;
		}
		if(flag==1){
			return i;
		}
	j=j+32;
	}
	return flag;
}

void executeProgram(char* name, int segment){
	int i;
	char buffer[13312];
	readFile(name,buffer);
	for (i=0; i<13312;i++){
		putInMemory(segment,i,buffer[i]);
	} 
	launchProgram(segment);
}

void terminate(){
	char shell[6];
	shell[0]='s';
	shell[1]='h';
	shell[2]='e';
	shell[3]='l';
	shell[4]='l';
	shell[5]='\0';
	interrupt(0x21,4, shell, 0x2000, 0);
}

void writeSector(char* buffer, int sector){
	int CH=intDiv(sector,36);
	int CL=mod(sector,18)+1;
	int DH=intDiv(sector,18);
	int DL=0;
	int CX=CH*256+CL;
	int DX=DH*256+DL;
	DH=mod(DH,2);
	interrupt(0x13, 3*256+1, buffer, CX, DX);
}

void deleteFile(char* name){
	int entryNo,fileSecsStart,j,k;
	char map[512];
	char dir[512];
	int sectors[27];
	readSector(map, 1);
  	readSector(dir, 2); 
	entryNo = exists(dir,name);
	if (entryNo==-1){
		printString("This file does not exist!\r\n");
		return;
	}	
	else{
		dir[entryNo*32]=0x00;
		fileSecsStart = entryNo*32+6;
		for (j=0;j<26;j++){
			sectors[j] = dir[fileSecsStart+j];
		}
		sectors[26] = 0;		
		
		for(k=0;k<26&&sectors[k]!=0;k++){
			map[sectors[k]] = 0x00;
		}

		writeSector(map,1);
		writeSector(dir,2);
	}
}

void writeFile(char* name,char* buffer, int secNum) {
	char map[512];
	char dir[512];
	char toBeWritten[512];
	int directoryEntry,length,l,diff,mapEntry,m,n,o,p;
	int* sectors;

	if(secNum>26){
		printString("You cannot use a value bigger than 26 for secNum!\r\n");
		return;
	}

	readSector(map,1);
	readSector(dir,2);
	
	directoryEntry=getDirectoryEntry(dir);

	if (directoryEntry == -1){
		printString("Directory is full!\r\n");
		return;
	}

	length=getLength(name);

	for (l=0;l<length;l++){
		dir[32*directoryEntry+l] = name[l];
	}

	if(length<6){
		diff=6-length;
		for(l=0;l<diff;l++){
			dir[directoryEntry*32+l+length]=0x0;
		}
	}

	for (m = 0; m < secNum; m++){
		mapEntry=getMapEntry(map);		
		
		if(mapEntry==-1){
			printString("Map is full!\r\n");
			return;
		}

		map[mapEntry]=0xFF;
		sectors[m]=mapEntry;
		dir[32*directoryEntry+6+m]=mapEntry;	
	}

	for(n=0;n<secNum;n++){
		for(o=0;o<512;o++){
			toBeWritten[o] = buffer[o*(n+1)];
		}
		writeSector(toBeWritten,sectors[n]);
	}

	for(p=0;p<(26-secNum);p++){
		dir[directoryEntry*32+31-p]=0x0;
	}

	writeSector(map,1);
	writeSector(dir,2);
}

int getDirectoryEntry(char* dir){
	int i;
     	for (i = 0; i < 16; i++){
		if (dir[i*32] == 0x00){
			return i;
		}
	}
	return -1;
}

int getLength(char* string){
	int length,i;
	length=0;
	for(i=0;string[i]!='\0';i++){
		length++;
	}
	return length;
}

int getMapEntry(char* map){
	int i;
	for(i=0;i<512;i++){
		if(map[i]==0x00){
			return i;
		}
	}
	return -1;
}

void getDirectory(char* buffer){
	int i,j;
	int k=0;
	int size=0;
	char dir[512];
	readSector(dir,2);
	for(i=0;i<16;i++){
		if (dir[i*32] != 0x00){
			for(j=0;j<6&&dir[i*32+j]!='\0';j++){
				buffer[k]=dir[i*32+j];
				k++;
			}
			size=0;
			for(j=6;j<26&&dir[i*32+j]!='\0';j++){
				size++;
			}
			buffer[k]=' ';
			k++;
			buffer[k]=intDiv(size,10)+'0';
			k++;
			buffer[k]=mod(size,10)+'0';
			k++;
			buffer[k]='\r';
			k++;
			buffer[k]='\n';
			k++;
		}
	}
}

void handleInterrupt21(int ax, int bx, int cx, int dx){
	switch(ax){
		case 0:printString(bx);break;
		case 1:readString(bx);break;
		case 2:readSector(bx,cx);break;
		case 3:readFile(bx,cx);break;
		case 4:executeProgram(bx,cx);break;
		case 5:terminate();break;
		case 6:writeSector(bx,cx);break;
		case 7:deleteFile(bx);break;
		case 8:writeFile(bx,cx,dx);break;
		case 9:getDirectory(bx);break;
		default:printString("Error");break;
	}
}
