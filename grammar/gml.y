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
%token NODE
%token EDGE

%token <ival> INT
%token <dval> FLOAT
%token <sval> ID
%token <sval> STRING_LITERAL

%type <attribute> kv;
%type <attribute> node;
%type <attribute> edge;
%type <attribute> item;
%type <attribute_list> kvs;
%type <attribute_list> items;
%type <graph> graph;

%%
graph:
    GRAPH LEFT_BRACKET items RIGHT_BRACKET 
    { $$ = new Graph($3); gml_graph = $$; delete $3; }
    ;

items:
    items item { $$ = $1; $1->push_back($2); }
    | 
    item { $$ = make_attribute_list($1); }
    ;

item:
    node { $$ = $1; } 
    | 
    edge { $$ = $1; }
    | 
    kv { $$ = $1; }
    ;

node: 
    NODE LEFT_BRACKET kvs RIGHT_BRACKET
    { $$ = new ListAttribute(AttributeType::NODE, "node", $3); }
    ;

edge: 
    EDGE LEFT_BRACKET kvs RIGHT_BRACKET
    { $$ = new ListAttribute(AttributeType::EDGE, "edge", $3); }
    ;

kvs:
    kvs kv  { $$ = $1; $1->push_back($2); }
    | 
    kv { $$ = make_attribute_list($1); }
    ;

kv:
    ID INT { $$ = new IntAttribute($1, $2); }
    |
    ID FLOAT { $$ = new FloatAttribute($1, $2); }
    |
    ID STRING_LITERAL { $$ = new StringAttribute($1 , $2); }
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