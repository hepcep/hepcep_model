%{
    #include <cstdio>
    #include <iostream>
    #include <vector>

    #include "gml.h"

    using namespace std;
    using namespace hepcep;

    extern int yylex();
    extern int yyparse();
    //extern FILE *yyin;
    extern int line_num;

    void yyerror(const char* s);

    Graph* gml_graph;

%}

%union {
    int ival;
    double dval;
    char* sval;

    hepcep::Graph* graph;
    hepcep::Attribute* attribute;
    std::vector<hepcep::Attribute*>* attribute_list;

}

%token LEFT_BRACKET
%token RIGHT_BRACKET
%token GRAPH

%token <ival> INT
%token <dval> FLOAT
%token <sval> ID
%token <sval> STRING_LITERAL

%type <attribute> kv;
%type <attribute> list;
%type <attribute_list> kvs;
%type <graph> graph;

%%
graph:
    GRAPH LEFT_BRACKET kvs RIGHT_BRACKET 
    { $$ = new Graph($3); gml_graph = $$; delete $3; }
    ;

list: 
    ID LEFT_BRACKET kvs RIGHT_BRACKET
    { $$ = make_list($1, $3); delete $3; }
    ;

kvs:
    kvs kv  { $$ = $1; $1->push_back($2); }
    | 
    kv { $$ = make_attribute_list($1); }
    ;

kv:
    ID INT { $$ = new IntAttribute($1, $2); free($1); }
    |
    ID FLOAT { $$ = new FloatAttribute($1, $2); free($1); }
    |
    ID STRING_LITERAL { $$ = new StringAttribute($1 , $2); free($1); free($2); }
    |
    list { $$ = $1; }
    ;

//value:
//    INT | FLOAT | STRING_LITERAL
//     ;

// key:
//     ID
//     ;
%%
// int main(int, char**) {
//   // open a file handle to a particular file:
//   FILE *myfile = fopen("../test_data/net_2.gml", "r");
//   // make sure it is valid:
//   if (!myfile) {
//     cout << "can't open file!" << endl;
//     return -1;
//   }
//   // Set flex to read from it instead of defaulting to STDIN:
//   yyin = myfile;
//   // Parse through the input:
//   yyparse();
//   gml_graph->evaluate();
// }

void yyerror(const char *s) {
  cout << "Error on line " << line_num << ". Message: " << s << endl;
  // might as well halt now:
  exit(-1);
}