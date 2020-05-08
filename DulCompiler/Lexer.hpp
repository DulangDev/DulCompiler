//
//  Lexer.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef Lexer_hpp
#define Lexer_hpp

#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <iostream>

class Lexem{
    
public:
    int lineno, linepos;
    enum type{
        NAME,
        INTLIT,
        FLOATLIT,
        STRLIT,
        ARROW,
        ASSIGN,
        PLUS,
        MINUS,
        ASTERISK,
        SLASH,
        DOT,
        OP_R_BR,
        CL_R_BR,
        OP_S_BR,
        CL_S_BR,
        OP_BRACE,
        CL_BRACE,
        EQ,
        NEQ,
        GEQ,
        LEQ,
        GREATER,
        LESS,
        COLON,
        COMMA,
        
        EOL,
        
        AS,
        KWFUN,
        KWCLASS,
        KWWRITE,
        KWRET,
        KWFOR,
        KWIN,
        KWWHILE,
        KWIF,
        KWELSE,
        KWTRUE,
        KWFALSE,
        KWOR,
        KWAND,
        KWNOT,
        KWINTERFACE,
        KWHAS,
        KWIMPORT,
        CLASS,
        END
        
    };
    
    static const char * typeRepr [];
    type lexType;
    union val_t{
        double fval;
        int64_t ival;
        char * strval;
    } val;
    int textlength;
    static const char * lexemText [];
    static const char * kwText [];
    
    bool isClosing() const{
        return lexType == END || lexType == CL_BRACE;
    }
    
    void print() const {
        printf("Lexem at %d:%d %s ", lineno, linepos, typeRepr[lexType]);
        if(lexType == NAME || lexType == STRLIT){
            std::cout << val.strval;
        }
        if( lexType == INTLIT ){
            std::cout<< val.ival;
        }
        if( lexType == FLOATLIT ){
            std::cout<< val.fval;
        }
        std::cout << std::endl;
    }
};
using lexIter = std::vector<Lexem>::iterator;
class LexemScanner{
    std::vector<Lexem> lexems;
    
    char * currpos;
    int lineno, linepos;
    char * source;
    char* scanLexem();
    char* scanString();
    void scanName();
    void scanNumber();
    
    
    
public:
    LexemScanner(const char * filename){
        FILE * f = fopen(filename, "r");
        size_t fsize;
        fseek(f, 0, SEEK_END);
        fsize = ftell(f);
        rewind(f);
        
        char * mem = (char*)malloc(fsize + 1);
        fread(mem, 1, fsize, f);
        mem[fsize] = 0;
        lineno = linepos = 0;
        currpos = mem;
        while(*currpos){
            currpos = scanLexem();
        }
        lexems.push_back(Lexem{
            lineno, linepos, Lexem::END, 0
        });
        
    }
    
    void print(){
        for(const auto& l: lexems){
            l.print();
        }
    }
    lexIter getIterator(){
        return lexems.begin();
    }
    
};

struct SyntaxError{
    int lineno, linepos;
    const char * message;
};





#endif /* Lexer_hpp */
