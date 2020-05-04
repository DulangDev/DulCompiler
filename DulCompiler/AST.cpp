//
//  AST.cpp
//  DulCompiler
//
//  Created by Дмитрий Маслюков on 18.04.2020.
//  Copyright © 2020 Дмитрий Маслюков. All rights reserved.
//

#include "AST.hpp"
#define COMP_ERR 1

AstNode* AstNode::parseFile(const char * name){
    LexemScanner scanner(name);
    scanner.print();
    lexIter iter = scanner.getIterator();
    try{
        return parseCompound(iter);
    } catch(int errcode){
        return 0;
    }
}



AstNode* AstNode::parseCompound(lexIter& iter){
    AstNode * block = new AstNode(BLOCK);
    while(not iter->isClosing()){
        try {
            AstNode * statement = parseStatement(iter);
            block->add(statement);
        } catch (SyntaxError error) {
            fprintf(stderr, "Syntax error at %d:%d\n%s\n", error.lineno, error.linepos, error.message);
            throw COMP_ERR;
        }
    }
    return block;
}
AstNode* AstNode::parseStatement(lexIter& iter){
    // a statement can be either
    // a write statement
    // an assign (further also inplace assign)
    // funccall or methodcall which is kind of funccall
    // also classdef interfacedef and funcdef are statements
    // upd if statement, while and for
    switch (iter->lexType) {
        case Lexem::KWWRITE:{
            AstNode * expr_part = parseExpr(iter);
            iter++;
            return new AstNode(WRITE, {expr_part});
        }
        case Lexem::EOL:{
            iter++;
        }break;
#warning TODO: for if, while, for
        default:{
            AstNode * lval = parseExpr(iter);
            Lexem next = *iter;
            if(next.lexType != Lexem::ASSIGN){
                throw SyntaxError{next.lineno, next.linepos, (char*)"expected ="};
            }
            iter++;
            AstNode* rval = parseExpr(iter);
            iter++;
            return new AstNode(ASSIGN, {lval, rval});
        }break;
    }
    throw SyntaxError{iter->lineno, iter->linepos, (char*)"statement required"};
    
    
}
AstNode* AstNode::parseExpr(lexIter& i){
    return parseLogOr(i);
}
AstNode* AstNode::parseLogOr(lexIter& i){
    AstNode * leftmost = parseLogAnd(i);
    while(i->lexType == Lexem::KWOR){
        i++;
        AstNode * righter = parseLogAnd(i);
        leftmost = new AstNode(OR, {leftmost, righter});
    }
    return leftmost;
}
AstNode* AstNode::parseLogAnd(lexIter& i){
    AstNode * leftmost = parseLogNot(i);
    while(i->lexType == Lexem::KWAND){
        i++;
        AstNode * righter = parseLogNot(i);
        leftmost = new AstNode(OR, {leftmost, righter});
    }
    return leftmost;
}
AstNode* AstNode::parseLogNot(lexIter& i){
    if(i->lexType == Lexem::KWNOT){
        i++;
        AstNode * expr = parseComp(i);
        return new AstNode(NOT, {expr});
    }
    return parseComp(i);
}
AstNode* AstNode::parseComp(lexIter& i){
    AstNode * leftmost = parseIn(i);
    while(i->lexType == Lexem::EQ || i->lexType == Lexem::LEQ || i->lexType == Lexem::LESS || i->lexType == Lexem::GEQ || i->lexType == Lexem::GREATER || i->lexType == Lexem::NEQ){
        Lexem::type t = i->lexType;
        i++;
        AstNode * righter = parseIn(i);
        leftmost = new AstNode(AstNode::type(t - Lexem::EQ+EQUALS), {leftmost, righter});
    }
    return leftmost;
}
AstNode* AstNode::parseIn(lexIter& i){
    AstNode * left = parsePlusMinus(i);
    if(i->lexType == Lexem::KWIN){
        i++;
        AstNode * right = parsePlusMinus(i);
        left = new AstNode(IN, {left, right});
    }
    return left;
}
AstNode* AstNode::parsePlusMinus(lexIter& i){
    AstNode * left = parseMultDiv(i);
    while( i->lexType == Lexem::PLUS || i->lexType == Lexem::MINUS ){
        Lexem::type t = i->lexType;
        i++;
        AstNode * right = parseMultDiv(i);
        left = new AstNode(AstNode::type(ADD + t - Lexem::PLUS), {left, right});
    }
    return left;
}
AstNode* AstNode::parseMultDiv(lexIter& i){
    AstNode * left = parseUnary(i);
    while( i->lexType == Lexem::ASTERISK || i->lexType == Lexem::SLASH ){
        Lexem::type t = i->lexType;
        i++;
        AstNode * right = parseUnary(i);
        left = new AstNode(AstNode::type(MUL + t - Lexem::ASTERISK), {left, right});
    }
    return left;
}
AstNode* AstNode::parseUnary(lexIter& i){
    switch (i->lexType) {
        case Lexem::PLUS:{
            i++;
            AstNode * expr = parseSuffix(i);
            return new AstNode(UPLUS, {expr});
        }break;
        case Lexem::MINUS:{
            i++;
            AstNode * expr = parseSuffix(i);
            return new AstNode(UMIN, {expr});
            
        }break;
        //therefore we know all field names of this we can omit explicit . operator
        /*case Lexem::DOT:{
            
        }break;*/
        default:
            return parseSuffix(i);
            break;
    }
}
AstNode* AstNode::parseSuffix(lexIter& i){
    AstNode * left = parseTopLevel(i);
    if(i->lexType == Lexem::OP_R_BR){
        i++;
        AstNode * expr = parseExpr(i);
        if(i->lexType != Lexem::CL_R_BR){
            throw SyntaxError{i->lineno, i->linepos, (char*)"expected ) bracket here"};
        }
        i++;
        return new AstNode(FCALL, {left, expr});
    } else if(i->lexType == Lexem::OP_S_BR){
        i++;
        AstNode * expr = parseExpr(i);
        if(i->lexType != Lexem::CL_S_BR){
            throw SyntaxError{i->lineno, i->linepos, (char*)"expected ] bracket here"};
        }
        i++;
        return new AstNode(FCALL, {left, expr});
    }
    return left;
}
AstNode* AstNode::parseTopLevel(lexIter& i){
    switch (i->lexType) {
        case Lexem::OP_R_BR:{
            i++;
            AstNode * expr = parseExpr(i);
            if(i->lexType != Lexem::CL_R_BR){
                throw SyntaxError{i->lineno, i->linepos, (char*)"expected ) bracket here"};
            }
            i++;
            return expr;
        }break;
        case Lexem::NAME:{
            
            AstNode * name = new AstNode(NAME);
            strcpy(name->memval, i->val.strval);
            i++;
            return name;
        }break;
        case Lexem::INTLIT:{
            
            AstNode * intlit = new AstNode(INTLIT);
            memcpy(intlit->memval, &i->val.ival, sizeof(int64_t));
            i++;
            return intlit;
        }break;
        case Lexem::FLOATLIT:{

            AstNode * intlit = new AstNode(FLOATLIT);
            memcpy(&intlit->memval, &i->val.fval, sizeof(double));
            i++;
            return intlit;
        }break;
        case Lexem::STRLIT:{
      
            AstNode * intlit = new AstNode(STRLIT);
            memcpy(&intlit->memval, &i->val.strval, sizeof(char*));
            i++;
            return intlit;
        }break;
        default:
            throw SyntaxError{i->lineno, i->linepos, (char*)"top level expection expected"};
            break;
    }
}
/*AstNode* AstNode::parseClassDef(lexIter&);
AstNode* AstNode::parseFuncDef(lexIter&);
AstNode* AstNode::parseInterface(lexIter&);*/

const char * AstNode::typerepr [] = {
    "RET",
    "WRITE",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "ASSIGN",
    "EQUALS",
    "NEQ",
    "GEQ",
    "LEQ",
    "GREATER",
    "LESS",
    
    "NOT",
    "AND",
    "OR",
    "IF",
    "WHILE",
    "FOR",
    "FUNCDEF",
    "CLASSDEF",
    "FCALL",
    "NAME",
    "STRLIT",
    "FLOATLIT",
    "INTLIT",
    "AT",
    "TYPEDECL",
    "TUPLE",
    "BLOCK",
    "IN",
    "UMIN",
    "UPLUS"
};
