#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

#define FileName_Size 256
#define UnicodeArray_Size 4

void GetFullPathToFiles(int argc, char* argv[], char PathToSrcFile[], char PathToDstFile[]);
void KeepOpenWindow();
FILE* OpenFile(char* mPathToFile, const char Mode[]);
int CloseFile(FILE* mStreamPointer);
void PerformConversion_CP1251_To_UTF8(FILE* StreamPointerSrc, FILE* StreamPointerDst);
void Convert_CP1251_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]);
void Convert_CP1251_To_UTF8_SpecialCharacters(const int NewChar, char Unicode[UnicodeArray_Size]);
size_t strlcpy(char *dst, const char *src, size_t dsize);

int main(int argc, char* argv[0])
{
    char PathToSrcFile[FileName_Size], PathToDstFile[FileName_Size];
    FILE* StreamPointerSrc;
    FILE* StreamPointerDst;
    int Result, CloseResult;
    _Bool OperationDone = true;

    GetFullPathToFiles(argc, argv, PathToSrcFile, PathToDstFile);

    StreamPointerSrc = OpenFile(PathToSrcFile, "rb");
    if(StreamPointerSrc == NULL){
        perror("Error open source file. Unknown file.\n");
        OperationDone = false;
    }

    StreamPointerDst = OpenFile(PathToDstFile, "wb");
    if(StreamPointerDst == NULL){
        perror("Error open destination file. Unknown file.\n");
        OperationDone = false;
    }

    // Выполняем конвертацию
    if(OperationDone == true)
        PerformConversion_CP1251_To_UTF8(StreamPointerSrc, StreamPointerDst);
    else
        Result = 1;

    // Если все прошло хорошо, закрываем файлы.
    CloseResult = CloseFile(StreamPointerSrc);
    if(CloseResult == EOF){
        perror("Error close source file.\n");
        OperationDone = false;
    };

    CloseResult = CloseFile(StreamPointerDst);
    if(CloseResult == EOF){
        perror("Error close destination file.\n");
        OperationDone = false;
    };

    if(OperationDone == true)
        printf("\nConversion done!");

    KeepOpenWindow();

    return Result;

}

// Метод выполняет конвертацию.
void PerformConversion_CP1251_To_UTF8(FILE* StreamPointerSrc, FILE* StreamPointerDst){

    int NewChar, InsertError;
    char Unicode[UnicodeArray_Size];

    while((NewChar = fgetc(StreamPointerSrc)) != EOF){

        Convert_CP1251_To_UTF8(NewChar, Unicode);
        InsertError = fputs(Unicode, StreamPointerDst);
        if(InsertError == EOF){
            perror("Writing error to the destination file.");
            exit(EXIT_FAILURE);
        }
    }
}

// Получаем адреса файлов.
void GetFullPathToFiles(int argc, char* argv[], char PathToSrcFile[], char PathToDstFile[]){

    // Обрабатываем аргументы командной строки (если они есть в "достаточном" количестве)
    if(argc == 3){
        strlcpy(PathToSrcFile, argv[1], FileName_Size - 1);
        strlcpy(PathToDstFile, argv[2], FileName_Size - 1);
    }
    else{

        // Если аргументы отсутсвует, работаем через интерфейс
        printf("        Welcome to the CP-1251 -> UTF-8 converter.\n");

        printf("Enter full path to the source file (max len - 250ch): ");
        scanf("%250s", PathToSrcFile);

        printf("Enter full path to the destination file (max len - 250ch): ");
        scanf("%250s", PathToDstFile);

    }
}

// Просто держит окно терминала открытым.
void KeepOpenWindow(){

    char EmptyEnter[1];
    scanf("%1s", EmptyEnter);
}

// Метод выполняет открытие файлов
FILE* OpenFile(char* mPathToFile, const char Mode[]) {

    struct stat buff;
    FILE* mStreamPointer = NULL;

    if(Mode[0] == 'w')
        mStreamPointer = fopen(mPathToFile, Mode);
    else{
        // Проверяем, что введенный путь указывает на обычный файл. Если это файл - попытаемся открыть.
        if(stat(mPathToFile, &buff) == 0 && (buff.st_mode & __S_IFMT) == __S_IFREG)
            mStreamPointer = fopen(mPathToFile, Mode);
    }

    return mStreamPointer;

}

// мметод закрывает файлы
int CloseFile(FILE* mStreamPointer) {

    int CloseResult;

    if(mStreamPointer == NULL){
        // Возвращаем 0, поскольку проверка корректности указателя должна быть выполнена заранее
        CloseResult = 0;
    }
    else
        CloseResult = fclose(mStreamPointer);

    return CloseResult;

}

size_t strlcpy(char *dst, const char *src, size_t dsize) {

	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1);	/* count does not include NUL */
}

// Приобразует символы кодировки CP-1251 в Unicode.
void Convert_CP1251_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]){

    switch(NewChar){

        case 0 ... 127:
            Unicode[0] = (char) NewChar;
            Unicode[1] = '\0';
            break;

        case 128 ... 191:
            Convert_CP1251_To_UTF8_SpecialCharacters(NewChar, Unicode);
            break;

        case 192 ... 239:
            Unicode[0] = 0xD0;
            Unicode[1] = (char)(NewChar - 48);
            Unicode[2] = '\0';
            break;

        case 240 ... 255:
            Unicode[0] = 0xD1;
            Unicode[1] = (char)(NewChar - 112);
            Unicode[2] = '\0';
            break;

        default:
            perror("Unknown symbol.");
            exit(EXIT_FAILURE);
    }
}

// Приобразует специальные символы кодировки CP-1251 в Unicode.
void Convert_CP1251_To_UTF8_SpecialCharacters(const int NewChar, char Unicode[UnicodeArray_Size]){

    switch(NewChar){

        case 128: // Ђ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x82;
            Unicode[2] = '\0';
            break;

        case 129: // Ѓ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x83;
            Unicode[2] = '\0';
            break;

        case 130: // ‚
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x9A;
            Unicode[3] = '\0';
            break;

        case 131: // ѓ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x93;
            Unicode[2] = '\0';
            break;

        case 132: // „
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x9E;
            Unicode[3] = '\0';
            break;

        case 133: // …
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xA6;
            Unicode[3] = '\0';
            break;

        case 134: // †
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xA0;
            Unicode[3] = '\0';
            break;

        case 135: // ‡
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xA1;
            Unicode[3] = '\0';
            break;

        case 136: // €
            Unicode[0] = 0xE2;
            Unicode[1] = 0x82;
            Unicode[2] = 0xAC;
            Unicode[3] = '\0';
            break;

        case 137: // ‰
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xB0;
            Unicode[3] = '\0';
            break;

        case 138: // Љ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x89;
            Unicode[2] = '\0';
            break;

        case 139: // ‹
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xB9;
            Unicode[3] = '\0';
            break;

        case 140: // Њ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x8A;
            Unicode[2] = '\0';
            break;

        case 141: // Ќ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x8C;
            Unicode[2] = '\0';
            break;

        case 142: // Ћ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x8B;
            Unicode[2] = '\0';
            break;

        case 143: // Џ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x8F;
            Unicode[2] = '\0';
            break;

        case 144: // ђ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x92;
            Unicode[2] = '\0';
            break;

        case 145: // ‘
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x98;
            Unicode[3] = '\0';
            break;

        case 146: // ’
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x99;
            Unicode[3] = '\0';
            break;

        case 147: // “
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x9C;
            Unicode[3] = '\0';
            break;

        case 148: // ”
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x9D;
            Unicode[3] = '\0';
            break;

         case 149: // •
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xA2;
            Unicode[3] = '\0';
            break;

        case 150: // –
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x93;
            Unicode[3] = '\0';
            break;

        case 151: // —
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0x94;
            Unicode[3] = '\0';
            break;

        case 152: //
            perror("Unknown special symbol.");
            exit(EXIT_FAILURE);
            break;

        case 153: // ™
            Unicode[0] = 0xE2;
            Unicode[1] = 0x84;
            Unicode[2] = 0xA2;
            Unicode[3] = '\0';
            break;

        case 154: // љ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x99;
            Unicode[2] = '\0';
            break;

        case 155: // ›
            Unicode[0] = 0xE2;
            Unicode[1] = 0x80;
            Unicode[2] = 0xBA;
            Unicode[3] = '\0';
            break;

        case 156: // њ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x9A;
            Unicode[2] = '\0';
            break;

        case 157: // ќ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x9C;
            Unicode[2] = '\0';
            break;

        case 158: // ћ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x9B;
            Unicode[2] = '\0';
            break;

        case 159: // џ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x9F;
            Unicode[2] = '\0';
            break;

        case 160: // <<Пробел>>
            Unicode[0] = 0x20;
            Unicode[1] = '\0';
            break;

        case 161: // Ў
            Unicode[0] = 0xD0;
            Unicode[1] = 0x8E;
            Unicode[2] = '\0';
            break;

        case 162: // ў
            Unicode[0] = 0xD1;
            Unicode[1] = 0x9E;
            Unicode[2] = '\0';
            break;

        case 163: // Ј
            Unicode[0] = 0xD0;
            Unicode[1] = 0x88;
            Unicode[2] = '\0';
            break;

        case 164: // ¤
            Unicode[0] = 0xC2;
            Unicode[1] = 0xA4;
            Unicode[2] = '\0';
            break;

        case 165: // Ґ
            Unicode[0] = 0xD2;
            Unicode[1] = 0x90;
            Unicode[2] = '\0';
            break;

        case 166: // ¦
            Unicode[0] = 0xC2;
            Unicode[1] = 0xA6;
            Unicode[2] = '\0';
            break;

        case 167: // §
            Unicode[0] = 0xC2;
            Unicode[1] = 0xA7;
            Unicode[2] = '\0';
            break;

        case 168: // Ё
            Unicode[0] = 0xD0;
            Unicode[1] = 0x81;
            Unicode[2] = '\0';
            break;

        case 169: // ©
            Unicode[0] = 0xC2;
            Unicode[1] = 0xA9;
            Unicode[2] = '\0';
            break;

        case 170: // Є
            Unicode[0] = 0xD0;
            Unicode[1] = 0x84;
            Unicode[2] = '\0';
            break;

        case 171: // «
            Unicode[0] = 0xC2;
            Unicode[1] = 0xAB;
            Unicode[2] = '\0';
            break;

        case 172: // ¬
            Unicode[0] = 0xC2;
            Unicode[1] = 0xAC;
            Unicode[2] = '\0';
            break;

        case 173: // ­
            Unicode[0] = 0xC2;
            Unicode[1] = 0xAD;
            Unicode[2] = '\0';
            break;

        case 174: // ®
            Unicode[0] = 0xC2;
            Unicode[1] = 0xAE;
            Unicode[2] = '\0';
            break;

        case 175: // Ї
            Unicode[0] = 0xD0;
            Unicode[1] = 0x87;
            Unicode[2] = '\0';
            break;

        case 176: // °
            Unicode[0] = 0xC2;
            Unicode[1] = 0xB0;
            Unicode[2] = '\0';
            break;

        case 177: // ±
            Unicode[0] = 0xC2;
            Unicode[1] = 0xB1;
            Unicode[2] = '\0';
            break;

        case 178: // І
            Unicode[0] = 0xD0;
            Unicode[1] = 0x86;
            Unicode[2] = '\0';
            break;

        case 179: // і
            Unicode[0] = 0xD1;
            Unicode[1] = 0x96;
            Unicode[2] = '\0';
            break;

        case 180: // ґ
            Unicode[0] = 0xD2;
            Unicode[1] = 0x91;
            Unicode[2] = '\0';
            break;

        case 181: // µ
            Unicode[0] = 0xC2;
            Unicode[1] = 0xB5;
            Unicode[2] = '\0';
            break;

        case 182: // ¶
            Unicode[0] = 0xC2;
            Unicode[1] = 0xB6;
            Unicode[2] = '\0';
            break;

        case 183: // ·
            Unicode[0] = 0xC2;
            Unicode[1] = 0xB7;
            Unicode[2] = '\0';
            break;

        case 184: // ё
            Unicode[0] = 0xD1;
            Unicode[1] = 0x91;
            Unicode[2] = '\0';
            break;

        case 185: // №
            Unicode[0] = 0xE2;
            Unicode[1] = 0x84;
            Unicode[2] = 0x96;
            Unicode[3] = '\0';
            break;

        case 186: // є
            Unicode[0] = 0xD1;
            Unicode[1] = 0x94;
            Unicode[2] = '\0';
            break;

        case 187: // »
            Unicode[0] = 0xC2;
            Unicode[1] = 0xBB;
            Unicode[2] = '\0';
            break;

        case 188: // ј
            Unicode[0] = 0xD1;
            Unicode[1] = 0x98;
            Unicode[2] = '\0';
            break;

        case 189: // Ѕ
            Unicode[0] = 0xD0;
            Unicode[1] = 0x85;
            Unicode[2] = '\0';
            break;

        case 190: // ѕ
            Unicode[0] = 0xD1;
            Unicode[1] = 0x95;
            Unicode[2] = '\0';
            break;

        case 191: // ї
            Unicode[0] = 0xD1;
            Unicode[1] = 0x97;
            Unicode[2] = '\0';
            break;
    }
}
