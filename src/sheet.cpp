#include "sheet.h"

#pragma comment(lib,"libxl.lib")

/*
 static void xls_test() {
	 BookHandle book = xlCreateBook();
	 if (book) {
		 int i, f[6];
		 FormatHandle format[6];
		 SheetHandle sheet;

		 f[0] = xlBookAddCustomNumFormat(book, L"0.0");
		 f[1] = xlBookAddCustomNumFormat(book, L"0.00");
		 f[2] = xlBookAddCustomNumFormat(book, L"0.000");
		 f[3] = xlBookAddCustomNumFormat(book, L"0.0000");
		 f[4] = xlBookAddCustomNumFormat(book, L"#,###.00 $");
		 f[5] = xlBookAddCustomNumFormat(book, L"#,###.00 $[Black][<1000];#,###.00 $[Red][>=1000]");

		 for (i = 0; i < 6; ++i) {
			 format[i] = xlBookAddFormat(book, 0);
			 xlFormatSetNumFormat(format[i], f[i]);
		 }

		 sheet = xlBookAddSheet(book, L"Custom formats", 0);
		 if (sheet){
			 xlSheetSetCol(sheet, 0, 0, 20, 0, 0);
			 xlSheetWriteNum(sheet, 2, 0, 25.718, format[0]);
			 xlSheetWriteNum(sheet, 3, 0, 25.718, format[1]);
			 xlSheetWriteNum(sheet, 4, 0, 25.718, format[2]);
			 xlSheetWriteNum(sheet, 5, 0, 25.718, format[3]);

			 xlSheetWriteNum(sheet, 7, 0, 1800.5, format[4]);

			 xlSheetWriteNum(sheet, 9, 0, 500, format[5]);
			 xlSheetWriteNum(sheet, 10, 0, 1600, format[5]);
		 }

		 if (xlBookSave(book, L"custom.xls")) printf("\nFile custom.xls has been created.\n");
		 xlBookRelease(book);
	 }

 }
*/
BookHandle sheet_open(const char *path, const char *name, const char *sheetname, int sheetodr, const char *cols[], int colcnt) {
  BookHandle book = xlCreateBook();
  if (!book) {
    sheet_printf("Can't Create Book : %s/%s\n", path, name);
    return NULL;
  }

  //wchar_t file[512];
  //wsprintf(file, L"%s/%s", path, sheet);

  char file[512];
  sprintf(file, "%s/%s", path, name);

  xlBookLoad(book, file);
  
  SheetHandle sh = NULL;
  sh = xlBookGetSheet(book, sheetodr);
  if (sh == NULL) {
    sh  = xlBookAddSheet(book, sheetname, 0);
    sheet_append_header(book, sheetodr, cols, colcnt);
  }
  
  return book;
}
void sheet_append(BookHandle bh, int odr, const char *cols[], int colcnt) {
  SheetHandle sh = xlBookGetSheet(bh, odr);
  int row = sheet_get_total_colnum(bh, odr);

  int i = 0;
  //xlSheetSetCol(sheet, 0, 0, 20, 0, 0);
  for (i = 0; i < colcnt; i++) {
    xlSheetWriteStr(sh, row, i, cols[i], 0);
  }
}
void sheet_append_header(BookHandle bh, int odr, const char *cols[], int colcnt) {
	SheetHandle sh = xlBookGetSheet(bh, odr);
	int row = sheet_get_total_colnum(bh, odr)+1;

	int i = 0;
	//xlSheetSetCol(sheet, 0, 0, 20, 0, 0);
	for (i = 0; i < colcnt; i++) {
		xlSheetWriteStr(sh, row, i, cols[i], 0);
	}
}
void sheet_save(BookHandle bh, const char *path, const char *sheet) {
  //wchar_t file[512];
  //wsprintf(file, L"%s/%s", path, sheet);

 char file[512];
 sprintf(file, "%s/%s", path, sheet);  
 xlBookSave(bh, file);
}
void sheet_close(BookHandle bh) {
  xlBookRelease(bh);
}

int sheet_get_total_colnum(BookHandle bh, int odr) {
	SheetHandle sh = xlBookGetSheet(bh, odr);
	return xlSheetLastRow(sh);
}

