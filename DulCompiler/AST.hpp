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
#include "Type.hpp"

struct AstNode{
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
        INTERFACE,
        SUBSCR,
        TYPECAST,
        FUNTYPEDECL
    };
    
    
    

    static const char * typerepr [];
    type t;
    char memval [32];
    std::vector<AstNode*> children;
    AstNode*parent = nullptr;
    int lineno, linepos;
    //self type as expression result
    Type * val_type;
    //table to lookup vars, etc
    LayoutType * namescope;
    LayoutType * outerscope;
public:
    void extracted();
    
    void inferTypes();
    void inferTypesAsObject();
    void removeRedundant();
    void setNameScope(LayoutType * ns, LayoutType * outerScope){
        namescope = ns;
        outerscope = outerScope;
        if(t != FUNCDEF)
        for(int i = 0; i < children.size(); i++){
            children[i]->setNameScope(ns, outerScope);
        }
        else{
            for(int i = 0; i < 4; i++){
                children[i]->setNameScope(ns, outerScope);
            }
        }
    }
    
    AstNode(type _t, std::initializer_list<AstNode*> _children={}){
        t = _t;
        children = _children;
        for(auto c :children){
            c->parent = this;
        }
        namescope = 0;
        val_type = 0;
    }
    
    void add(AstNode* child){
        children.push_back(child);
        if(child)
        child->parent = this;
    }
    
    bool isNumLiteral() const{
        return t == INTLIT || t == FLOATLIT;
    }
    
    LayoutType * getNamescope() {
        return namescope;
    }
    
    void parseFundecl(LayoutType * os);
    void inferClassDeclTypes();
    void parseMethodDecl(Type * this_type);
    
    virtual void print(int offt, std::ostream &ostream= std::cout) const{
        for(int i = 0; i < offt; i++){
            ostream << " ";
        }
        ostream << typerepr[t] << " " << std::hex << (long)this << std::dec << " ";
    
        if(val_type)
            ostream << "type: " <<val_type->name << " ";
        if(t == NAME || t == STRLIT){
            ostream << memval;
        }
        if(t == INTLIT){
            ostream << (*(int64_t*)memval);
        }
        if(t == FLOATLIT){
             ostream << (*(double*)memval);
        }
        ostream<<std::endl;
        if(t != FUNCDEF)
        for(const auto & c:children){
            c->print(offt + 4, ostream);
        }
        else{
            for(int i = 0; i < 4; i++)
                children[i]->print(offt + 4, ostream);
            for(int i = 0; i < offt+4; i++){
                ostream << " ";
            }
            ostream << "Functional parent: " <<std::hex <<(long)children[4] << std::dec << std::endl;
            for(int i = 0; i < offt+4; i++){
                ostream << " ";
            } ostream << "closures:" << std::endl;
            if(children.size() > 3)
            children[5]->print(offt+8, ostream);
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
    static AstNode* parseAs(lexIter&);
    
};

struct SemanticError{
    int lineno, linepos;
    const char * message;
    
    virtual void print(std::ostream & ostream){
        ostream << "Semantic error at "<<lineno << ":" << linepos << "\n" << message;
    }
};

struct TypeError: public SemanticError{
    
    TypeError(Type * _expected, Type * _got){
        expected = _expected;
        got = _got;
    }
    
    Type * expected, *got;
    void print (std::ostream & ostream) override{
        ostream << "Type error at "<<lineno<<":"<<linepos<<"\n"<< expected->name << "expected but got" << got->name;
    }
};
void setFunctionParenthesisDownwalk(AstNode* node, AstNode * f = 0);

#endif /* AST_hpp */
