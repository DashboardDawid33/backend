#include <stdio.h>
#include <signal.h>
#include <stdio.h>

int callback(void *NotUsed,int argc ,char **argv,char **azColName) {
    int i;
    for(i=0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}