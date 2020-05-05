//
//  AST.hpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#ifndef AST_hpp
#define AST_hpp

#include <stdio.h>
#include <vector>
#include "Lexer.hpp"

class AstNode{
public:
    enum type{
        RET,
        WRITE,
        ADD,
        SUB,
        MUL,
        DIV,
        ASSIGN,
        EQUALS,
        NEQ,
        GEQ,
        LEQ,
        GREATER,
        LESS,
       
        NOT,
        AND,
        OR,
        IF,
        WHILE,
        FOR,
        FUNCDEF,
        CLASSDEF,
        FCALL,
        NAME,
        STRLIT,
        FLOATLIT,
        INTLIT,
        AT,
        TYPEDECL,
        TUPLE,
        BLOCK,
        IN,
        UMIN,
        UPLUS,
        EMPTY,
        _TRUE,
        _FALSE,
        INTERFACE
    };
    
    
    
private:
    static const char * typerepr [];
    type t;
    char memval [32];
    std::vector<AstNode*> children;
    int lineno, linepos;
public:
    AstNode(type _t, std::initializer_list<AstNode*> _children={}){
        t = _t;
        children = _children;
    }
    
    void add(AstNode* child){
        children.push_back(child);
    }
    
    void print(int offt) const{
        for(int i = 0; i < offt; i++){
            std::cout << " ";
        }
        printf("%s ", typerepr[t] /*,lineno, linepos*/);
        if(t == NAME || t == STRLIT){
            printf("%s ", memval);
        }
        if(t == INTLIT){
            printf("%lld", *(int64_t*)memval);
        }
        if(t == FLOATLIT){
            printf("%lf", *(double*)memval);
        }
        std::cout<<std::endl;
        for(const auto & c:children){
            c->print(offt + 4);
        }
    }
    
    static AstNode* parseFile(const char * name);
private:
    static AstNode* parseCompound(lexIter&);
    static AstNode* parseStatement(lexIter&);
    static AstNode* parseExpr(lexIter&);
    static AstNode* parseLogOr(lexIter&);
    static AstNode* parseLogAnd(lexIter&);
    static AstNode* parseLogNot(lexIter&);
    static AstNode* parseComp(lexIter&);
    static AstNode* parseIn(lexIter&);
    static AstNode* parsePlusMinus(lexIter&);
    static AstNode* parseMultDiv(lexIter&);
    static AstNode* parseUnary(lexIter&);
    static AstNode* parseSuffix(lexIter&);
    static AstNode* parseTopLevel(lexIter&);
    static AstNode* parseClassDef(lexIter&);
    static AstNode* parseFuncDef(lexIter&);
    static AstNode* parseInterface(lexIter&);
    static AstNode* parseIf(lexIter&);
    static AstNode* parseFor(lexIter&);
    static AstNode* parseWhile(lexIter&);
    
};

#endif /* AST_hpp */
