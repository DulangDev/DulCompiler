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
    } catch(SyntaxError err){
        fprintf(stderr, "Syntax error at %d:%d  %s\n", err.lineno, err.linepos, err.message);
        return 0;
    }
}




AstNode* AstNode::parseCompound(lexIter& iter){
    AstNode * block = new AstNode(BLOCK);
    while(not iter->isClosing()){
            AstNode * statement = parseStatement(iter);
            block->add(statement);
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
            return new AstNode(EMPTY);
        }break;
        case Lexem::KWRET:{
            iter++;
            AstNode * expression;
            if(iter->lexType != Lexem::EOL)
                expression = parseExpr(iter);
            else
                expression = new AstNode(EMPTY);
            iter++;
            return new AstNode(RET, {expression});
            
        }
        case Lexem::KWIF:{
            iter++;
            return parseIf(iter);
        }break;
        case Lexem::KWWHILE:{
            iter++;
            return parseWhile(iter);
        }break;
        case Lexem::KWFOR:{
            iter++;
            return parseFor(iter);
        }break;
        case Lexem::KWCLASS:{
            iter++;
            return parseClassDef(iter);
        }break;
        case Lexem::KWINTERFACE:{
            iter++;
            return parseInterface(iter);
        }break;
        case Lexem::KWFUN:{
            iter++;
            return parseFuncDef(iter);
        }
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
    AstNode * first = parseLogOr(i);
    if(i->lexType == Lexem::COMMA){
        first = new AstNode(TUPLE, {first});
        while(i->lexType == Lexem::COMMA){
            i++;
            AstNode * cur = parseLogOr(i);
            first->add(cur);
        }
    }
    return first;
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
    } else if(i->lexType == Lexem::DOT){
        i++;
        AstNode * rhs = parseSuffix(i);
        return new AstNode(AT, {left, rhs});
    } else if(i->lexType == Lexem::COLON){
        i++;
        AstNode * type = parseSuffix(i);
        return new AstNode(TYPEDECL, {left, type});
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
        case Lexem::KWTRUE:{
            i++;
            return new AstNode(_TRUE);
        }break;
        case Lexem::KWFALSE:{
            i++;
            return new AstNode(_FALSE);
        }break;
        default:
            throw SyntaxError{i->lineno, i->linepos, (char*)"top level expection expected"};
            break;
    }
}
AstNode* AstNode::parseClassDef(lexIter& i){
    AstNode * classdef = new AstNode(CLASSDEF);
    AstNode * name = parseTopLevel(i);
    classdef->add(name);
    if(i->lexType == Lexem::KWHAS){
        i++;
        AstNode * extending = parseExpr(i);
        classdef->add(extending);
    }
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { at class declaration"};
    }
    i++;
    AstNode * block = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } at end of class declaration"};
    }
    i++;
    classdef->add(block);
    return classdef;
}
AstNode* AstNode::parseFuncDef(lexIter& i){
    //kyeword fun is skipped already
    AstNode * fundecl = new AstNode(FUNCDEF);
    AstNode * name = parseTopLevel(i);
    if(i->lexType != Lexem::OP_R_BR){
        throw SyntaxError{i->lineno, i->linepos, "( expected at function declaration"};
    }
    i++;
    AstNode * args;
    if(i->lexType != Lexem::CL_R_BR)
         args = parseExpr(i);
    else
        args = new AstNode(EMPTY);
    if(i->lexType != Lexem::CL_R_BR){
        throw SyntaxError{i->lineno, i->linepos, ") expected after parameter list"};
    }
    fundecl->add(name);
    fundecl->add(args);
    i++;
    
    if(i->lexType != Lexem::OP_BRACE){
        //think of it as abstract fundecl
        return fundecl;
    }
    i++;
    i++;
    AstNode * body = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } at end of function body"};
    }
    i++;
    fundecl->add(body);
    return fundecl;
}


AstNode* AstNode::parseInterface(lexIter&i){
    AstNode * idef = new AstNode(INTERFACE);
    AstNode * name = parseTopLevel(i);
    idef->add(name);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { at class declaration"};
    }
    i++;
    AstNode * block = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } at end of class declaration"};
    }
    i++;
    idef->add(block);
    return idef;
}

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
    "UPLUS",
    "NOP",
    "TRUE",
    "FALSE",
    "INTERFACE"
};

AstNode* AstNode::parseIf(lexIter& i){
    AstNode * cond_expr = parseExpr(i);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { after if statement"};
    }
    i++;
    AstNode * if_block = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } after if body statement"};
    }
    i++;
    AstNode * ifstat = new AstNode(IF, {cond_expr, if_block});
    if(i->lexType == Lexem::KWELSE){
        i++;
        if(i->lexType != Lexem::OP_BRACE){
            throw SyntaxError{i->lineno, i->linepos, "expected { after else statement"};
        }
        AstNode * elseblock = parseCompound(i);
        if(i->lexType != Lexem::CL_BRACE){
            throw SyntaxError{i->lineno, i->linepos, "expected } after else body statement"};
        }
        i++;
        ifstat->add(elseblock);
        
    }
    return ifstat;
}
AstNode* AstNode::parseFor(lexIter&i){
    AstNode * iter_name = parseTopLevel(i);
    if(i->lexType != Lexem::KWIN){
        throw SyntaxError{i->lineno, i->linepos, "expected in after for iterator name"};
    }
    i++;
    AstNode * coll = parseExpr(i);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { after for"};
    }
    i++;
    AstNode * body = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } after for"};
    }
    i++;
    return new AstNode(FOR, {iter_name, coll, body});
}
AstNode* AstNode::parseWhile(lexIter&i){
    AstNode * while_expr = parseExpr(i);
    if(i->lexType != Lexem::OP_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected { after while"};
    }
    i++;
    AstNode * body = parseCompound(i);
    if(i->lexType != Lexem::CL_BRACE){
        throw SyntaxError{i->lineno, i->linepos, "expected } after while"};
    }
    i++;
    return new AstNode(WHILE, {while_expr, body});
}
