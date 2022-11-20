//**************************************************************************
 
// purpose: This is the logic behind the parser, with lots of delicate code.
// version: Fall 2023
//  author: Corbin Rochelle

//**************************************************************************
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <iostream>
#include "parser.h"
 
using namespace std;

int nextToken = 0;            // hold nextToken returned by lex

// Which tree level are we currently in?  
static int level = 0;

// Feel free to use a different data structure for the symbol table (list of
// variables declared in the program) but you will have to adjust the code in
// main() to print out the symbol table after a successful parse


//*****************************************************************************
// Indent to reveal tree structure
string psp(void) { // Stands for p-space, but I want the name short
  string str("");
  for(int i = 0; i < level; i++)
    str += "|  ";
  return str;
}
//*****************************************************************************
// Report what we found
void output(string what) {
  cout << psp() << "found |" << yytext << "| " << what << endl;
}
//*****************************************************************************
// Forward declarations of FIRST_OF functions.  These check whether the current 
// token is in the FIRST set of a production rule.
bool first_of_program(void);

void lex() {
    nextToken = yylex();
}
bool first_of_program(void) {
    return nextToken == TOK_PROGRAM;
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <program> → TOK_PROGRAM TOK_IDENT TOK_SEMICOLON <block>
ProgramNode* program()
{
    if (!first_of_program()) // Check for PROGRAM
        throw "3: 'PROGRAM' expected";
    
    if (nextToken != TOK_PROGRAM) throw("3: 'PROGRAM' expected");
    output("PROGRAM");
    
    cout << psp() << "enter <program>" << endl;
    ++level;
    ProgramNode* newProgramNode = new ProgramNode(level);

    lex();
    output("IDENTIFIER");
    lex();
    if (nextToken != TOK_SEMICOLON) throw ("14: ';' expected");
    output("SEMICOLON");
    lex();
    output("BLOCK");
    newProgramNode->block = block();

    --level;
    cout << psp() << "exit <program>" << endl;
    lex();
    
    return newProgramNode;
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <block> → stuff
BlockNode* block() {
    string ident, typer;
    
    cout << psp() << "enter <block>" << endl;
    ++level;
    BlockNode* newBlockNode = new BlockNode(level);
    
    if (nextToken != TOK_VAR && nextToken != TOK_BEGIN) throw("18: error in declaration part OR 17: 'BEGIN' expected");
    bool checker = true;
    if (nextToken == TOK_BEGIN) {
        checker = false;    }
    else lex();
    
   
    while(checker) {
        if (nextToken != TOK_IDENT) throw ("2: identifier expected");
        output("IDENTIFIER");
        ident = string(yytext);
        lex();
        if (nextToken != TOK_COLON) throw ("5: ':' expected");
        output("COLON");
        lex();
        if (nextToken != TOK_INTEGER && nextToken != TOK_REAL) throw ("10: error in type");
        output("TYPE");
        typer = yytext;
        lex();
        if (nextToken != TOK_SEMICOLON) throw ("14: ';' expected");
        output("SEMICOLON");
        lex();
        cout << psp() << "-- idName: |" << ident << "| idType: |" << typer << "| --" << endl;
        if (inSymbolTable(ident)) {
            yytext = (char *)";";
            throw ("101: identifier declared twice");
        }
        symbolTable.insert(pair<string, float>(ident,0));

        if (nextToken == TOK_BEGIN) {
            checker = false;
        }
    }
    
    checker = true;
    newBlockNode->statement = statement();
    while(checker){
        if (nextToken == TOK_END) {
            checker = false;
            break;
        }
        if (nextToken != TOK_SEMICOLON) throw("14: ';' expected");
        output("SEMICOLON");
        newBlockNode->statement = statement();
    }
    
    --level;
    cout << psp() << "exit <block>" << endl;
    
    return newBlockNode;
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <statement> → stuff
StatementNode* statement() {
    bool skipper = false;
    bool looper = true;
    switch(nextToken) {
        case TOK_BEGIN: {
            output("BEGIN");
            cout << psp() << "enter <compound_stmt>" << endl;
            ++level;
            CompoundStmtNode* newCompoundStmtNode = new CompoundStmtNode(level);
            
            while(looper){
                lex();
                output("STATEMENT");
                newCompoundStmtNode->restStatements.push_back(statement());
                if (nextToken != TOK_SEMICOLON) looper = false;
                if (looper) output("SEMICOLON");
            }
            
            --level;
            if (nextToken != TOK_END) {
                throw("13: 'END' expected");
            }
            output("END");
            cout << psp() << "exit <compound_stmt>" << endl;
            lex();
            if (strcmp(yytext,"") == 0) nextToken = TOK_END;
            
            return newCompoundStmtNode;
        }
            
        case TOK_IF: {
            cout << psp() << "enter <if>" << endl;
            ++level;
            IfStmtNode* newIfStmtNode = new IfStmtNode(level);
            
            lex();
            output("EXPRESSION");
            newIfStmtNode->expression = expression();
            if (nextToken != TOK_THEN) throw("52: 'THEN' expected");
            output("THEN");
            lex();
            output("STATEMENT");
            newIfStmtNode->firstStatement = statement();
            if (nextToken == TOK_ELSE) skipper = true;
            if (skipper) {
                --level;
                output("ELSE");
                cout << psp() << "enter <else>" << endl;
                ++level;
                lex();
                output("STATEMENT");
                newIfStmtNode->secondStatement = statement();
            }
            --level;
            cout << psp() << "exit <if>" << endl;
            
            return newIfStmtNode;
        }
        
            
        case TOK_WHILE: {
            cout << psp() << "enter <while>" << endl;
            ++level;
            WhileStmtNode* newWhileStmtNode = new WhileStmtNode(level);
            
            lex();
            output("EXPRESSION");
            newWhileStmtNode->expression = expression();
            output("STATEMENT");
            newWhileStmtNode->statement = statement();
            --level;
            cout << psp() << "exit <while>" << endl;
            
            return newWhileStmtNode;
        }
            
        case TOK_READ: {
            cout << psp() << "enter <read>" << endl;
            ++level;
            ReadStmtNode* newReadStmtNode = new ReadStmtNode(level);
            
            lex();
            if (nextToken != TOK_OPENPAREN) throw("9: '(' expected");
            output("OPENPAREN");
            lex();
            if (!inSymbolTable(yytext)) throw ("104: identifier not declared");
            output("IDENTIFIER");
            cout << psp() << yytext << endl;
            newReadStmtNode->_input = yytext;
            lex();
            if (nextToken != TOK_CLOSEPAREN) throw("4: ')' expected");
            output("CLOSEPAREN");
            lex();
            
            --level;
            cout << psp() << "exit <read>" << endl;
            return newReadStmtNode;
        }
            
        case TOK_WRITE: {
            cout << psp() << "enter <write>" << endl;
            ++level;
            WriteStmtNode* newWriteStmtNode = new WriteStmtNode(level);
            
            lex();
            if (nextToken != TOK_OPENPAREN) throw("9: '(' expected");
            output("OPENPAREN");
            lex();
            output("WRITE");
            cout << psp() << yytext << endl;
            newWriteStmtNode->_input = yytext;
            lex();
            if (nextToken != TOK_CLOSEPAREN) throw("4: ')' expected");
            output("CLOSEPAREN");
            lex();
            
            --level;
            cout << psp() << "exit <write>" << endl;
            return newWriteStmtNode;
        }
            
        case TOK_IDENT: {
            cout << psp() << "enter <assignment>" << endl;
            ++level;
            AssignmentStmtNode* newAssignmentStmtNode = new AssignmentStmtNode(level);
            
            if (!inSymbolTable(string(yytext))) throw ("104: identifier not declared");
            output("IDENTIFIER");
            cout << psp() << yytext << endl;
            newAssignmentStmtNode->_var = yytext;
            lex();
            if(nextToken != TOK_ASSIGN) throw("51: ':=' expected");
            output("ASSIGN");
            lex();
            output("EXPRESSION");
            newAssignmentStmtNode->expression = expression();
            
            --level;
            cout << psp() << "exit <assignment>" << endl;
            return newAssignmentStmtNode;
        }
        default:
            throw("900: illegal type of statement");
    }
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <expression> → stuff
ExpressionNode* expression() {
    cout << psp() << "enter <expression>" << endl;
    ++level;
    ExpressionNode* newExpressionNode = new ExpressionNode(level);
    
    output("SIMPLE_EXP");
    newExpressionNode->simpleExpression = simple_expression();
    
    bool checker = false;
    switch(nextToken) {
        case TOK_EQUALTO:
            output("EQUALTO");
            cout << psp() << yytext << endl;
            newExpressionNode->restExpressionOps.push_back(yytext);
            checker = true;
            break;
        case TOK_LESSTHAN:
            output("LESSTHAN");
            cout << psp() << yytext << endl;
            newExpressionNode->restExpressionOps.push_back(yytext);
            checker = true;
            break;
        case TOK_GREATERTHAN:
            output("GREATERTHAN");
            cout << psp() << yytext << endl;
            newExpressionNode->restExpressionOps.push_back(yytext);
            checker = true;
            break;
        case TOK_NOTEQUALTO:
            output("NOTEQUALTO");
            cout << psp() << yytext << endl;
            newExpressionNode->restExpressionOps.push_back(yytext);
            checker = true;
            break;
    }
    
    if(checker) {
        lex();
        output("SIMPLE_EXP");
        newExpressionNode->simpleExpression2 = simple_expression();
    }
    
    
    --level;
    cout << psp() << "exit <expression>" << endl;
    return newExpressionNode;
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <simple_expression> → stuff
SimpleExpressionNode* simple_expression() {
    cout << psp() << "enter <simple_exp>" << endl;
    ++level;
    SimpleExpressionNode* newSimpleExpressionNode = new SimpleExpressionNode(level);
    
    output("TERM");
    newSimpleExpressionNode->firstTerm = term();
    
    bool outerloop = true;
    
    while(outerloop) {
        bool checker = false;
        switch(nextToken) {
            case TOK_PLUS: {
                output("PLUS");
                cout << psp() << yytext << endl;
                newSimpleExpressionNode->restSimpleExpressionOps.push_back(yytext);
                checker = true;
                lex();
                break;
            }
            case TOK_MINUS: {
                output("MINUS");
                cout << psp() << yytext << endl;
                newSimpleExpressionNode->restSimpleExpressionOps.push_back(yytext);
                checker = true;
                lex();
                break;
            }
            case TOK_OR: {
                output("OR");
                newSimpleExpressionNode->restSimpleExpressionOps.push_back(yytext);
                checker = true;
                lex();
                break;
            }
        }
        
        if(checker) {
            output("TERM");
            newSimpleExpressionNode->restTerms.push_back(term());
        }
        
        if (nextToken != TOK_PLUS || nextToken != TOK_MINUS || nextToken != TOK_OR) outerloop = false;
    }
    
    --level;
    cout << psp() << "exit <simple_exp>" << endl;
    return newSimpleExpressionNode;
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <term> → stuff
TermNode* term() {
    cout << psp() << "enter <term>" << endl;
    ++level;
    TermNode* newTermNode = new TermNode(level);
    
    output("FACTOR");
    newTermNode->firstFactor = factor();
    
    bool outerloop = true;
    
    lex();
    while(outerloop) {
        switch(nextToken) {
            case TOK_MULTIPLY: {
                output("MULTIPLY");
                cout << psp() << yytext << endl;
                newTermNode->restTermOps.push_back(yytext);
                lex();
                output("FACTOR");
                newTermNode->restFactor.push_back(factor());
                lex();
                break;
            }
            case TOK_DIVIDE: {
                output("DIVIDE");
                cout << psp() << yytext << endl;
                newTermNode->restTermOps.push_back(yytext);
                lex();
                output("FACTOR");
                newTermNode->restFactor.push_back(factor());
                lex();
                break;
            }
            case TOK_AND: {
                output("AND");
                newTermNode->restTermOps.push_back(yytext);
                lex();
                output("FACTOR");
                newTermNode->restFactor.push_back(factor());
                lex();
                break;
            }
        }
        
        if (nextToken != TOK_MULTIPLY && nextToken != TOK_DIVIDE && nextToken != TOK_AND) outerloop = false;
    }
    
    --level;
    cout << psp() << "exit <term>" << endl;
    return newTermNode;
}
//*****************************************************************************
// Parses strings in the language generated by the rule:
// <factor> → stuff
FactorNode* factor() {
    cout << psp() << "enter <factor>" << endl;
    ++level;
    
    switch(nextToken) {
        case TOK_INTLIT: {
            output("INTLIT");
            cout << psp() << yytext << endl;
            IntNode* newIntNode = new IntNode(level, yytext);
            --level;
            cout << psp() << "exit <factor>" << endl;
            return newIntNode;
        }
        case TOK_FLOATLIT: {
            output("FLOATLIT");
            cout << psp() << yytext << endl;
            FloatNode* newFloatNode = new FloatNode(level, stof(yytext));
            --level;
            cout << psp() << "exit <factor>" << endl;
            return newFloatNode;
        }
        case TOK_IDENT: {
            output("IDENTIFIER");
            cout << psp() << yytext << endl;
            IdentNode* newIdentNode = new IdentNode(level, yytext);
            if (!inSymbolTable(string(yytext))) throw ("104: identifier not declared");
            --level;
            cout << psp() << "exit <factor>" << endl;
            return newIdentNode;
        }
        case TOK_OPENPAREN: {
            output("OPENPAREN");
            cout << psp() << yytext << endl;
            lex();
            output("EXPRESSION");
            NestedExpNode* newNestedExpNode = new NestedExpNode(level);
            newNestedExpNode->expression = expression();
            if (nextToken != TOK_CLOSEPAREN) throw("4: ')' expected");
            output("CLOSEPAREN");
            --level;
            cout << psp() << "exit <factor>" << endl;
            return newNestedExpNode;
        }
        case TOK_NOT: {
            output("NOT");
            cout << psp() << yytext << endl;
            lex();
            output("FACTOR");
            NotNode* newNotNode = new NotNode(level);
            newNotNode->factor = factor();
            --level;
            cout << psp() << "exit <factor>" << endl;
            return newNotNode;
        }
        case TOK_MINUS: {
            output("MINUS");
            cout << psp() << yytext << endl;
            lex();
            output("FACTOR");
            MinusNode* newMinusNode = new MinusNode(level);
            newMinusNode->factor = factor();
            --level;
            cout << psp() << "exit <factor>" << endl;
            return newMinusNode;
        }
        default:
            throw("903: illegal type of factor");
    }
    
    --level;
    cout << psp() << "exit <factor>" << endl;
}
