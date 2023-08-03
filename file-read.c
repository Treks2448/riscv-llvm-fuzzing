#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

/* #include <clang-c/Index.h> */

#define uint32_1s uint32_t 0xffffffff
#define uint32_half_1s uint32_t 0x0000ffff
#define uint64_1s uint64_t 0xffffffffffffffff
#define uint64_half_1s uint64_t 0x00000000ffffffff

/*
void print_cursor(CXCursor cursor, int depth) {
	for (int i = 0; i < depth; i++) {
		printf("  ");
	}

	CXString kindName = clang_getCursorKindSpelling(clang_getCursorKind(cursor));
	CXString cursorSpelling = clang_getCursorSpelling(cursor);
	printf("%s: %s\n", clang_getCString(kindName), clang_getCString(cursorSpelling));
}

enum CXChildVisitResult visit(CXCursor cursor, CXCursor parent, CXClientData client_data) {
	int* depth = (int*)client_data;
	print_cursor(cursor, *depth);

	(*depth)++;
	clang_visitChildren(cursor, visit, depth);
	(*depth)--;

	return CXChildVisit_Continue;
}
*/

void afl_custom_fuzz(uint8_t *buf, size_t buf_size) {
	struct CXUnsavedFile virt_file  = {
		.Filename = "virt_file.c",
		.Contents = (char *)buf,	
		.Length = buf_size
	};

	CXIndex idx = clang_createIndex(0, 0);
	CXTranslationUnit unit = clang_parseTranslationUnit(
		idx, "virt_file.c",
		NULL, 0,
		&virt_file, 1,
		CXTranslationUnit_None);
	if (unit == NULL) {
		printf("Unable to parse translation unit\n");
		return;
	}

	CXCursor cursor = clang_getTranslationUnitCursor(unit);
	int depth = 0;
	//clang_visitChildren(cursor, visit, &depth);

	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(idx);
}

int main(int argc, char** argv) {
	FILE *file = NULL;
	long length = -1;

	file = fopen("test.c", "r");
	if (file == NULL) {
		printf("Failed to open the file\n");	
		return 1;
	}

	if (fseek(file, 0L, SEEK_END) != 0) {
		printf("Failed to seek to the end of the file\n");
		return 1;
	}
	
	length = ftell(file);
	if (length == -1) {
		printf("Failed to tell the position in the file stream\n");
		return 1;
	}

	rewind(file);

	// Make sure to terminate with 0 (length might need to be +1)	
	char file_buf[length+1];
	fread(file_buf, sizeof(*file_buf), length, file);
	if (ferror(file) != 0) {
		printf("An error occured while reading from the file\n");
		return 1;
	}
	file_buf[length] = 0;

	afl_custom_fuzz((uint8_t*)file_buf, (size_t)length);

	fclose(file);

	return 0;
}
