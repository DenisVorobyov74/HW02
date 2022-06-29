#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

#define FileName_Size 256
#define UnicodeArray_Size 4
#define CodingModeArray_Size 11

void GetFullPathToFiles(int argc, char* argv[], char PathToSrcFile[], char PathToDstFile[], char CodingMode[11]);
void KeepOpenWindow();
FILE* OpenFile(char* mPathToFile, const char Mode[]);
int CloseFile(FILE* mStreamPointer);
void PerformConversion(void (*ConversionFnctn)(const int, char*), FILE* StreamPointerSrc, FILE* StreamPointerDst);
void Convert_CP1251_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]);
void Convert_KOI8R_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]);
void Convert_ISO_8859_5_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]);
size_t strlcpy(char *dst, const char *src, size_t dsize);

int main(int argc, char* argv[0])
{
    char PathToSrcFile[FileName_Size], PathToDstFile[FileName_Size], CodingMode[CodingModeArray_Size];
    FILE* StreamPointerSrc;
    FILE* StreamPointerDst;
    int Result, CloseResult;
    _Bool OperationDone = true;
    //Объявляем указатель на функцию
    void (*ConversionFnctn)(const int, char*) = NULL;


    GetFullPathToFiles(argc, argv, PathToSrcFile, PathToDstFile, CodingMode);

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
    if(OperationDone == true){
        if(strncmp(CodingMode, "CP-1251", CodingModeArray_Size-1) == 0)
            ConversionFnctn = Convert_CP1251_To_UTF8;
        else if(strncmp(CodingMode, "KOI8-R", CodingModeArray_Size-1) == 0)
            ConversionFnctn = Convert_KOI8R_To_UTF8;
        else if(strncmp(CodingMode, "ISO-8859-5", CodingModeArray_Size-1) == 0)
            ConversionFnctn = Convert_ISO_8859_5_To_UTF8;
        else{
            perror("Unknown encoding format.\n");
            exit(EXIT_FAILURE);
        }
        PerformConversion(ConversionFnctn, StreamPointerSrc, StreamPointerDst);
    }
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
void PerformConversion(void (*ConversionFnctn)(const int, char*), FILE* StreamPointerSrc, FILE* StreamPointerDst){

    int NewChar, InsertError;
    char Unicode[UnicodeArray_Size];

    while((NewChar = fgetc(StreamPointerSrc)) != EOF){

        ConversionFnctn(NewChar, Unicode);
        InsertError = fputs(Unicode, StreamPointerDst);
        if(InsertError == EOF){
            perror("Writing error to the destination file.");
            exit(EXIT_FAILURE);
        }
    }
}

// Получаем адреса файлов.
void GetFullPathToFiles(int argc, char* argv[], char PathToSrcFile[], char PathToDstFile[], char CodingMode[11]){

    // Обрабатываем аргументы командной строки (если они есть в "достаточном" количестве)
    if(argc == 4){
        strlcpy(PathToSrcFile, argv[1], FileName_Size - 1);
        strlcpy(PathToDstFile, argv[2], FileName_Size - 1);
        strlcpy(CodingMode,    argv[3], 10);
    }
    else{

        // Если аргументы отсутсвует, работаем через интерфейс
        printf("        Welcome to the CP-1251 -> UTF-8 converter.\n");

        printf("Enter full path to the source file (max len - 250ch): ");
        scanf("%250s", PathToSrcFile);

        printf("Enter full path to the destination file (max len - 250ch): ");
        scanf("%250s", PathToDstFile);

        printf("Enter coding mode (CP-1251, KOI8-R or ISO-8859-5) (max len - 10ch): ");
        scanf("%10s", CodingMode);

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

    static char SpecCharArray[64][4] = {{0xD0,0x82,'\0'},      {0xD0, 0x83, '\0'},    {0xE2,0x80,0x9A,'\0'}, {0xD1,0x93,'\0'},      // Ђ, Ѓ, ‚, ѓ
                                        {0xE2,0x80,0x9E,'\0'}, {0xE2,0x80,0xA6,'\0'}, {0xE2,0x80,0xA0,'\0'}, {0xE2,0x80,0xA1,'\0'}, // „, …, †, ‡
                                        {0xE2,0x82,0xAC,'\0'}, {0xE2,0x80,0xB0,'\0'}, {0xD0,0x89,'\0'},      {0xE2,0x80,0xB9,'\0'}, // €, ‰, Љ, ‹
                                        {0xD0,0x8A,'\0'},      {0xD0,0x8C,'\0'},      {0xD0,0x8B,'\0'},      {0xD0,0x8F,'\0'},      // Њ, Ќ, Ћ, Џ
                                        {0xD1,0x92,'\0'},      {0xE2,0x80,0x98,'\0'}, {0xE2,0x80,0x99,'\0'}, {0xE2,0x80,0x9C,'\0'}, // ђ, ‘, ’, “
                                        {0xE2,0x80,0x9D,'\0'}, {0xE2,0x80,0xA2,'\0'}, {0xE2,0x80,0x93,'\0'}, {0xE2,0x80,0x94,'\0'}, // ”, •, –, —
                                        {'\0','\0','\0','\0'}, {0xE2,0x84,0xA2,'\0'}, {0xD1,0x99,'\0'},      {0xE2,0x80,0xBA,'\0'}, // , ™, љ, ›
                                        {0xD1,0x9A,'\0'},      {0xD1,0x9C,'\0'},      {0xD1,0x9B,'\0'},      {0xD1,0x9F,'\0'},      // њ, ќ, ћ, џ
                                        {0x20,'\0'},           {0xD0,0x8E,'\0'},      {0xD1,0x9E,'\0'},      {0xD0,0x88,'\0'},      // <<Пробел>>, Ў, ў, Ј
                                        {0xC2,0xA4,'\0'},      {0xD2,0x90,'\0'},      {0xC2,0xA6,'\0'},      {0xC2,0xA7,'\0'},      // ¤, Ґ, ¦, §
                                        {0xD0,0x81,'\0'},      {0xC2,0xA9,'\0'},      {0xD0,0x84,'\0'},      {0xC2,0xAB,'\0'},      // Ё, ©, Є, «
                                        {0xC2,0xAC,'\0'},      {0xC2,0xAD,'\0'},      {0xC2,0xAE,'\0'},      {0xD0,0x87,'\0'},      // ¬, , ®, Ї
                                        {0xC2,0xB0,'\0'},      {0xC2,0xB1,'\0'},      {0xD0,0x86,'\0'},      {0xD1,0x96,'\0'},      // °, ±, І, і
                                        {0xD2,0x91,'\0'},      {0xC2,0xB5,'\0'},      {0xC2,0xB6,'\0'},      {0xC2,0xB7,'\0'},      // ґ, µ, ¶, ·
                                        {0xD1,0x91,'\0'},      {0xE2,0x84,0x96,'\0'}, {0xD1,0x94,'\0'},      {0xC2,0xBB,'\0'},      // ё, №, є, »
                                        {0xD1,0x98,'\0'},      {0xD0,0x85,'\0'},      {0xD1,0x95,'\0'},      {0xD1,0x97,'\0'}};     // ј, Ѕ, ѕ, ї

    switch(NewChar){

        case 0 ... 127:
            Unicode[0] = (char) NewChar;
            Unicode[1] = '\0';
            break;

        case 128 ... 191:
            if(NewChar == 152){
                perror("Unknown special symbol.");
                exit(EXIT_FAILURE);
            }
            else
                strlcpy(Unicode, SpecCharArray[NewChar-128], UnicodeArray_Size);
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

// Приобразует символы кодировки KOI8-R в Unicode.
void Convert_KOI8R_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]){

    static char CharArray[128][4] = {{0xE2,0x94,0x80,'\0'}, {0xE2,0x94,0x82,'\0'}, {0xE2,0x94,0x8C,'\0'}, {0xE2,0x94,0x90,'\0'}, // ─, │, ┌, ┐
                                     {0xE2,0x94,0x94,'\0'}, {0xE2,0x94,0x98,'\0'}, {0xE2,0x94,0x9C,'\0'}, {0xE2,0x94,0xA4,'\0'}, // └, ┘, ├, ┤
                                     {0xE2,0x94,0xAC,'\0'}, {0xE2,0x94,0xB4,'\0'}, {0xE2,0x94,0xBC,'\0'}, {0xE2,0x96,0x80,'\0'}, // ┬, ┴, ┼, ▀
                                     {0xE2,0x96,0x84,'\0'}, {0xE2,0x96,0x88,'\0'}, {0xE2,0x96,0x8C,'\0'}, {0xE2,0x96,0x90,'\0'}, // ▄, █, ▌, ▐
                                     {0xE2,0x96,0x91,'\0'}, {0xE2,0x96,0x92,'\0'}, {0xE2,0x96,0x93,'\0'}, {0xE2,0x8C,0xA0,'\0'}, // ░, ▒, ▓, ⌠
                                     {0xE2,0x96,0xA0,'\0'}, {0xE2,0x88,0x99,'\0'}, {0xE2,0x88,0x9A,'\0'}, {0xE2,0x89,0x88,'\0'}, // ■, ∙, √, ≈
                                     {0xE2,0x89,0xA4,'\0'}, {0xE2,0x89,0xA5,'\0'}, {0x20,'\0'},           {0xE2,0x8C,0xA1,'\0'}, // ≤, ≥, <Пробел>>, ⌡
                                     {0xC2,0xB0,'\0'},      {0xC2,0xB2,'\0'},      {0xC2,0xB7,'\0'},      {0xC3,0xB7,'\0'},      // °, ², ·, ÷
                                     {0xE2,0x95,0x90,'\0'}, {0xE2,0x95,0x91,'\0'}, {0xE2,0x95,0x92,'\0'}, {0xD1,0x91,'\0'},      // ═, ║, ╒, ё
                                     {0xE2,0x95,0x93,'\0'}, {0xE2,0x95,0x94,'\0'}, {0xE2,0x95,0x95,'\0'}, {0xE2,0x95,0x96,'\0'}, // ╓, ╔, ╕, ╖
                                     {0xE2,0x95,0x97,'\0'}, {0xE2,0x95,0x98,'\0'}, {0xE2,0x95,0x99,'\0'}, {0xE2,0x95,0x9A,'\0'}, // ╗, ╘, ╙, ╚
                                     {0xE2,0x95,0x9B,'\0'}, {0xE2,0x95,0x9C,'\0'}, {0xE2,0x95,0x9D,'\0'}, {0xE2,0x95,0x9E,'\0'}, // ╛, ╜, ╝, ╞
                                     {0xE2,0x95,0x9F,'\0'}, {0xE2,0x95,0xA0,'\0'}, {0xE2,0x95,0xA1,'\0'}, {0xD0,0x81,'\0'},      // ╟, ╠, ╡, Ё
                                     {0xE2,0x95,0xA2,'\0'}, {0xE2,0x95,0xA3,'\0'}, {0xE2,0x95,0xA4,'\0'}, {0xE2,0x95,0xA5,'\0'}, // ╢, ╣, ╤, ╥
                                     {0xE2,0x95,0xA6,'\0'}, {0xE2,0x95,0xA7,'\0'}, {0xE2,0x95,0xA8,'\0'}, {0xE2,0x95,0xA9,'\0'}, // ╦, ╧, ╨, ╩
                                     {0xE2,0x95,0xAA,'\0'}, {0xE2,0x95,0xAB,'\0'}, {0xE2,0x95,0xAC,'\0'}, {0xC2,0xA9,'\0'},      // ╪, ╫, ╬, ©
                                     {0xD1,0x8E,'\0'},      {0xD0,0xB0,'\0'},      {0xD0,0xB1,'\0'},      {0xD1,0x86,'\0'},      // ю, а, б, ц
                                     {0xD0,0xB4,'\0'},      {0xD0,0xB5,'\0'},      {0xD1,0x84,'\0'},      {0xD0,0xB3,'\0'},      // д, е, ф, г
                                     {0xD1,0x85,'\0'},      {0xD0,0xB8,'\0'},      {0xD0,0xB9,'\0'},      {0xD0,0xBA,'\0'},      // х, и, й, к
                                     {0xD0,0xBB,'\0'},      {0xD0,0xBC,'\0'},      {0xD0,0xBD,'\0'},      {0xD0,0xBE,'\0'},      // л, м, н, о
                                     {0xD0,0xBF,'\0'},      {0xD1,0x8F,'\0'},      {0xD1,0x80,'\0'},      {0xD1,0x81,'\0'},      // п, я, р, с
                                     {0xD1,0x82,'\0'},      {0xD1,0x83,'\0'},      {0xD0,0xB6,'\0'},      {0xD0,0xB2,'\0'},      // т, у, ж, в
                                     {0xD1,0x8C,'\0'},      {0xD1,0x8B,'\0'},      {0xD0,0xB7,'\0'},      {0xD1,0x88,'\0'},      // ь, ы, з, ш
                                     {0xD1,0x8D,'\0'},      {0xD1,0x89,'\0'},      {0xD1,0x87,'\0'},      {0xD1,0x8A,'\0'},      // э, щ, ч, ъ, Ю
                                     {0xD0,0xAE,'\0'},      {0xD0,0x90,'\0'},      {0xD0,0x91,'\0'},      {0xD0,0xA6,'\0'},      // Ю, А, Б, Ц
                                     {0xD0,0x94,'\0'},      {0xD0,0x95,'\0'},      {0xD0,0xA4,'\0'},      {0xD0,0x93,'\0'},      // Д, Е, Ф, Г
                                     {0xD0,0xA5,'\0'},      {0xD0,0x98,'\0'},      {0xD0,0x99,'\0'},      {0xD0,0x9A,'\0'},      // Х, И, Й, К
                                     {0xD0,0x9B,'\0'},      {0xD0,0x9C,'\0'},      {0xD0,0x9D,'\0'},      {0xD0,0x9E,'\0'},      // Л, М, Н, О
                                     {0xD0,0x9F,'\0'},      {0xD0,0xAF,'\0'},      {0xD0,0xA0,'\0'},      {0xD0,0xA1,'\0'},      // П, Я, Р, С
                                     {0xD0,0xA2,'\0'},      {0xD0,0xA3,'\0'},      {0xD0,0x96,'\0'},      {0xD0,0x92,'\0'},      // Т, У, Ж, В
                                     {0xD0,0xAC,'\0'},      {0xD0,0xAB,'\0'},      {0xD0,0x97,'\0'},      {0xD0,0xA8,'\0'},      // Ь, Ы, З, Ш
                                     {0xD0,0xAD,'\0'},      {0xD0,0xA9,'\0'},      {0xD0,0xA7,'\0'},      {0xD0,0xAA,'\0'}};     // Э, Щ, Ч, Ъ

    switch(NewChar){

        case 0 ... 127:
            Unicode[0] = (char) NewChar;
            Unicode[1] = '\0';
            break;

        case 128 ... 255:
            strlcpy(Unicode, CharArray[NewChar-128], UnicodeArray_Size);
            break;

        default:
            perror("Unknown symbol.");
            exit(EXIT_FAILURE);
    }
}

// Приобразует символы кодировки ISO-8859-5 в Unicode.
void Convert_ISO_8859_5_To_UTF8(const int NewChar, char Unicode[UnicodeArray_Size]){

    switch(NewChar){

        case 0 ... 127:
            Unicode[0] = (char) NewChar;
            Unicode[1] = '\0';
            break;

        case 128 ... 159:
            perror("Unknown symbol.");
            exit(EXIT_FAILURE);
            break;

        case 160:               // <<НеразрывныйПробел>>
            Unicode[0] = 0x20;
            Unicode[1] = '\0';
            break;

        case 161 ... 223:       // Ё, Ђ, Ѓ, Є, Ѕ, І, Ї, Ј, Љ, Њ, Ћ, Ќ, ,Ў, Џ; А...Я; а...п
            Unicode[0] = 0xD0;
            Unicode[1] = (char)(NewChar - 32);
            Unicode[2] = '\0';
            break;

        case 224 ... 239:       // р...я
            Unicode[0] = 0xD1;
            Unicode[1] = (char)(NewChar - 96);
            Unicode[2] = '\0';
            break;

         case 240:              // №
            Unicode[0] = 0xE2;
            Unicode[1] = 0x84;
            Unicode[2] = 0x96;
            Unicode[3] = '\0';
            break;

         case 241 ... 252:      // ё, ђ, ѓ, є, ѕ, і, ї, ј, љ, њ, ћ, ќ
            Unicode[0] = 0xD1;
            Unicode[1] = (char)(NewChar - 96);
            Unicode[2] = '\0';
            break;

        case 253:              // §
            Unicode[0] = 0xC2;
            Unicode[1] = 0xA7;
            Unicode[2] = '\0';
            break;

        case 254 ... 255:      // ў, џ
            Unicode[0] = 0xD1;
            Unicode[1] = (char)(NewChar - 96);
            Unicode[2] = '\0';
            break;

        default:
            perror("Unknown symbol.");
            exit(EXIT_FAILURE);
    }
}
