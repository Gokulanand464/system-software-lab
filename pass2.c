#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXBUF 1024
#define MAXTEXT 60
int main() {
    FILE *f1, *f2, *f3, *f4, *f5, *f6;
    int i, len, tlen = 0;
    char l1[10], address[10], label[10], opcode[10], operand[20], text_start[10];
    char code[10], mne[10], sym[10], add[10], start[10];
    char objcode[MAXBUF] = "";
    f1 = fopen("intermediate.txt", "r");
    f2 = fopen("length.txt", "r");
    f3 = fopen("symtab.txt", "r");
    f4 = fopen("opcode.txt", "r");
    f5 = fopen("assembly.txt", "w");
    f6 = fopen("obj.txt", "w");

    if (!f1 || !f2 || !f3 || !f4 || !f5 || !f6) {
        printf("Error opening files.\n");
        return 1;
    }
    fscanf(f1, "%s%s%s", label, opcode, operand);
    fprintf(f5, "%s\t%s\t%s\n", label, opcode, operand);

    if (strcmp(opcode, "START") == 0) {
        strcpy(start, operand);
        fscanf(f2, "%s", l1);
        fprintf(f6, "H^%-6s^00%s^00%s\n", label, start, l1);
        fscanf(f1, "%s%s%s%s", address, label, opcode, operand);
    }
    strcpy(text_start, address);
    strcpy(objcode, "");
    tlen = 0;
    while (strcmp(opcode, "END") != 0) {
        int found = 0;
        char temp[100] = "";
        rewind(f4);
        while (fscanf(f4, "%s%s", code, mne) != EOF) {
            if (strcmp(opcode, code) == 0) {
                rewind(f3);
                while (fscanf(f3, "%s%s", add, sym) != EOF) {
                    if (strcmp(operand, sym) == 0) {
                        sprintf(temp, "%s%s", mne, add);
                        fprintf(f5, "%s\t%s\t%s\t%s\t%s\n", address, label, opcode, operand, temp);
                        found = 1;
                        break;
                    }
                }
                break;
            }
        }
        if (!found) {
            if (strcmp(opcode, "WORD") == 0) {
                sprintf(temp, "%06X", atoi(operand));
                fprintf(f5, "%s\t%s\t%s\t%s\t%s\n", address, label, opcode, operand, temp);
            } else if (strcmp(opcode, "BYTE") == 0) {
                if (operand[0] == 'C') {
                    for (i = 2; i < strlen(operand) - 1; i++) {
                        char hex[5];
                        sprintf(hex, "%02X", operand[i]);
                        strcat(temp, hex);
                    }
                } else if (operand[0] == 'X') {
                    strncpy(temp, operand + 2, strlen(operand) - 3);
                    temp[strlen(operand) - 3] = '\0';
                } 
                fprintf(f5, "%s\t%s\t%s\t%s\t%s\n", address, label, opcode, operand, temp);
            } else if (strcmp(opcode, "RESW") == 0 || strcmp(opcode, "RESB") == 0) {
                if (tlen > 0) {
                    fprintf(f6, "T^00%s^%02X^%s\n", text_start, tlen, objcode);
                    strcpy(objcode, "");
                    tlen = 0;
                }
                strcpy(text_start, address);
                fprintf(f5, "%s\t%s\t%s\t%s\n", address, label, opcode, operand);
                fscanf(f1, "%s%s%s%s", address, label, opcode, operand);
                continue;
            } else {
                fprintf(f5, "%s\t%s\t%s\t%s\n", address, label, opcode, operand);
            }
        }

        // If we got an object code, append it
        if (strlen(temp) > 0) {
            if (tlen + strlen(temp) / 2 > 30) { // max 30 bytes per text record
                fprintf(f6, "T^00%s^%02X^%s\n", text_start, tlen, objcode);
                strcpy(objcode, "");
                strcpy(text_start, address);
                tlen = 0;
            }
            strcat(objcode, temp);
            strcat(objcode, "^");
            tlen += strlen(temp) / 2;
        }

        fscanf(f1, "%s%s%s%s", address, label, opcode, operand);
    }

    // Flush last text record
    if (tlen > 0) {
        fprintf(f6, "T^00%s^%02X^%s\n", text_start, tlen, objcode);
    }

    // End record
    fprintf(f6, "E^00%s\n", start);
    fprintf(f5, "%s\t%s\t%s\t%s\n", address, label, opcode, operand);

    fclose(f1);
    fclose(f2);
    fclose(f3);
    fclose(f4);
    fclose(f5);
    fclose(f6);

//    printf("\n---Object program written to obj.txt---\n");
//    printf("---Assembly listing written to assembly.txt---\n");

    return 0;
}

