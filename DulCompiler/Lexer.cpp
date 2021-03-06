//
//  Lexer.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "Lexer.hpp"
#include <ctype.h>

const char * Lexem::lexemText []= {
    "->",
    "=",
    "+",
    "-",
    "*",
    "/",
    ".",
    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    "==",
    "!=",
    ">=",
    "<=",
    ">",
    "<",
    ":",
    ",",
    
    "\n"
};
const char * Lexem::kwText [] = {
    "as",
    "fun",
    "class",
    "write",
    "return",
    "for",
    "in",
    "while",
    "if",
    "else",
    "true",
    "false",
    "or",
    "and",
    "not",
    "interface",
    "has",
    "import",
    "class"
};



char* LexemScanner::scanLexem(){
    char * scanpos = currpos;
    
    while(*scanpos == ' ' || *scanpos=='\t'){
        scanpos++;
    }
    
    if(*scanpos == '"'){
        return scanString();
    }
    
    for(int i = 0; i < sizeof(Lexem::lexemText)/sizeof(char*); i++){
        if(strncmp(Lexem::lexemText[i], scanpos, strlen(Lexem::lexemText[i])) == 0){
            Lexem l;
            l.lexType = (Lexem::type)(i + Lexem::ARROW);
            if(i + Lexem::ARROW == Lexem::EOL){
                lineno++;
                linepos = 0;
            }
            l.textlength = int(strlen(Lexem::lexemText[i]) + scanpos - currpos);
            l.lineno = lineno;
            l.linepos = linepos;
            linepos += l.textlength;
            currpos += l.textlength;
            lexems.push_back(l);
            return currpos;
        }
    }
    
    if(isalpha(*scanpos)){
        //scan name or keyword
        const char * nameend = scanpos;
        while(isalpha(*nameend)){
            nameend++;
        }
        int len = int(nameend - currpos);
        linepos += len;
        currpos += len;
        for(int i = 0; i < sizeof(Lexem::kwText)/sizeof(char*); i++){
            
            
            bool is_s_eq = nameend - scanpos == strlen(Lexem::kwText[i]);
            if(strncmp(scanpos, Lexem::kwText[i], strlen(Lexem::kwText[i])) == 0 && is_s_eq){
                lexems.push_back(Lexem{
                    lineno, linepos - len,
                    (Lexem::type)(i+(int)Lexem::AS),
                    0,
                    len
                });
                return currpos;
            }
        }
        lexems.push_back(Lexem{
            .lineno = lineno,
            .linepos = linepos-len,
            .lexType = Lexem::NAME,
            .val.strval = strndup(scanpos, nameend - scanpos),
            .textlength = len
        });
    }
#ifndef isnumber
#define isnumber isdigit
#endif
    if(isnumber(*scanpos)){
        char * endnum;
        double val = strtod(scanpos, &endnum);
        int len = int(endnum - currpos);
        currpos += len;
        linepos += len;
        if(val == (long long)val){
            lexems.push_back(Lexem{
                .lineno =lineno,
                .linepos =linepos - len,
                .lexType = Lexem::INTLIT,
                .val.ival = (int64_t)val,
                .textlength = len
            });
            return currpos;
        }
        lexems.push_back(Lexem{
            .lineno =lineno,
            .linepos =linepos - len,
            .lexType = Lexem::FLOATLIT,
            .val.fval = val,
            .textlength = len
        });
        return currpos;
    }
    return currpos;
    
    
}

char * LexemScanner::scanString(){
    //assume that currpos is "
    char * strend = currpos + 1;
    while(*strend && *strend != '"'){
        strend++;
    }
    
    lexems.push_back(Lexem{
        .lineno =lineno,
        .linepos =linepos,
        .lexType = Lexem::STRLIT,
        .val.strval = strndup(currpos, strend - currpos+1),
        .textlength = static_cast<int>(strend - currpos+1)
    });
    linepos  += strend+1 - currpos;
    return strend+1;
}

const char * Lexem::typeRepr [] = {
    "NAME",
    "INTLIT",
    "FLOATLIT",
    "STRLIT",
    "ARROW",
    "Assign",
    "PLUS",
    "MINUS",
    "ASTERISK",
    "SLASH",
    "DOT",
    "OP_R_BR",
    "CL_R_BR",
    "OP_S_BR",
    "CL_S_BR",
    "OP_BRACE",
    "CL_BRACE",
    "EQ",
    "NEQ",
    "GEQ",
    "LEQ",
    "GREATER",
    "LESS",
    "COLON",
    "COMMA",
    
    "EOL",
    "AS",
    "KWFUN",
    "KWCLASS",
    "KWWRITE",
    "KWRET",
    "KWFOR",
    "KWIN",
    "KWWHILE",
    "KWIF",
    "KWELSE",
    "KWOR",
    "KWAND",
    "KWNOT",
    "KWTRUE",
    "KWFALSE",
    "KWINTERFACE",
    "KWHAS",
    "KWIMPORT",
    "CLASS",
    "END"
};
