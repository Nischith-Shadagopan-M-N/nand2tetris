#include <iostream>
#include <fstream>
#include <stack>
#include <vector>
#include <string>
#include <bitset>
#include <map>
#include <sstream>

using namespace std;

int labelNum;

class jackTokenizer {
public:
	ifstream jackfile;
	ofstream indent;
	string currToken;
	string prevToken;
	vector <string> className;
	void init(string name) {
		jackfile.open(name);
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.push_back('t');
		name.push_back('x');
		name.push_back('t');
		indent.open(name);
		char c;
		while (jackfile.get(c)) {
			if (c == '/') {
				char temp = c;
				jackfile.get(c);
				if (c == '/') {
					string s;
					getline(jackfile, s);
				}
				else if (c == '*') {
					char tem;
					do {
						tem = c;
						jackfile.get(c);
					} while (tem != '*' || c != '/');
				}
				else {
					indent << " ";
					indent << temp;
					indent << " ";
					indent << c;
				}
			}
			else if (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == '.' || c == ',' || c == ';' || c == '+' || c == '-' || c == '*' || c == '&' || c == '|' || c == '<' || c == '>' || c == '=' || c == '~') {
				indent << " ";
				indent << c;
				indent << " ";
			}
			else {
				indent << c;
			}
		}
		jackfile.close();
		indent.close();
		jackfile.open(name);
	}
	bool hasMoreTokens() {
		if (jackfile.eof()) {
			return 0;
		}
		else {
			return 1;
		}
	}
	int advance() {
		string temp = currToken;
		jackfile >> currToken;
		if (currToken[0] == '"') {
			char c;
			jackfile.get(c);
			while (c != '"') {
				if(c == '\n'){
					cerr<<"Invalid stringConstant"<<endl;
				}
				currToken.push_back(c);
				jackfile.get(c);
			}
		}
		if (currToken.size() == 0) {
			cerr<<"Expected more tokens"<<endl;
			return -1;
		}
		else if (tokenType() == "identifier") {
			if (prevToken == "class") {
				className.push_back(currToken);
			}
		}
		if (temp.size() > 0) {
			prevToken = temp;
		}
	}
	bool nextToken() {
		while (hasMoreTokens()) {
			if (advance() != -1) {
				return 1;
			}
			else {
				continue;
			}
		}
		return 0;
	}
	string tokenType(string tok = "") {
		string check;
		if (tok == "") {
			check = currToken;
		}
		else {
			check = tok;
		}
		if (check == "class" || check == "constructor" || check == "function" || check == "method" || check == "field" || check == "static" || check == "var" || check == "int" || check == "char" || check == "boolean" || check == "void" || check == "true" || check == "false" || check == "null" || check == "this" || check == "let" || check == "do" || check == "if" || check == "else" || check == "while" || check == "return") {
			return "keyword";
		}
		if (check == "{" || check == "}" || check == "(" || check == ")" || check == "[" || check == "]" || check == "." || check == "," || check == ";" || check == "+" || check == "-" || check == "*" || check == "&" || check == "|" || check == "<" || check == ">" || check == "=" || check == "~" || check == "/") {
			return "symbol";
		}
		bool b = 1;
		for(auto e : check){
			b = b && isdigit(e);
		}
		if (b) {
			return "integerConstant";
		}
		b = 1;
		if (check[0] == '"') {
			return "stringConstant";
		}
		for(auto e : check){
			b = b && (isdigit(e)||(e=='_')||isalpha(e));
		}
		if(b && !isdigit(check[0])){
			return "identifier";
		}
		cerr<<"Invalid token: "<<check<<endl;
		return "";
	}
	int intVal() {
		stringstream s(currToken);
		int x;
		s >> x;
		return x;
	}
	string stringVal() {
		return currToken.substr(1, currToken.size() - 1);
	}
};

class identifier {
public:
	string type, kind;
	int number;
	identifier(string t = "", string k = "", int n = 0) {
		type = t;
		kind = k;
		number = n;
	}
};

class SymbolTable {
public:
	int staticCounter, fieldCounter, argCounter, varCounter;
	map <string, identifier> classScope, subroutineScope;
	string currClass, currSubroutine, currSubroutineType;
	void init(string name) {
		classScope.clear();
		subroutineScope.clear();
		staticCounter = 0;
		fieldCounter = 0;
		argCounter = 0;
		varCounter = 0;
		currClass = name;
	}
	void startSubroutine(string name, string type) {
		subroutineScope.clear();
		currSubroutine = name;
		currSubroutineType = type;
		argCounter = 0;
		varCounter = 0;
	}
	void define(string name, string type, string kind) {
		if (kind == "static") {
			identifier temp{ type, kind, staticCounter };
			staticCounter++;
			classScope[name] = temp;
		}
		else if (kind == "field") {
			identifier temp{ type, kind, fieldCounter };
			fieldCounter++;
			classScope[name] = temp;
		}
		else if (kind == "arg") {
			identifier temp{ type, kind, argCounter };
			argCounter++;
			subroutineScope[name] = temp;
		}
		else {
			identifier temp{ type, kind, varCounter };
			varCounter++;
			subroutineScope[name] = temp;
		}
	}
	int varCount(string kind) {
		if (kind == "static") {
			return staticCounter;
		}
		if (kind == "field") {
			return fieldCounter;
		}
		if (kind == "arg") {
			return argCounter;
		}
		else {
			return varCounter;
		}
	}
	string kindOf(string name) {
		if (classScope.count(name)) {
			if (classScope[name].kind == "field")
				return "this";
			return classScope[name].kind;
		}
		if (subroutineScope.count(name)) {
			if (subroutineScope[name].kind == "var")
				return "local";
			else
				return "argument";
		}
		return "NONE";
	}
	string typeOf(string name) {
		if (classScope.count(name)) {
			return classScope[name].type;
		}
		if (subroutineScope.count(name)) {
			return subroutineScope[name].type;
		}
		return "NONE";
	}
	int indexOf(string name) {
		if (classScope.count(name)) {
			return classScope[name].number;
		}
		if (subroutineScope.count(name)) {
			return subroutineScope[name].number;
		}
		return -1;
	}
};

SymbolTable s;

class vmWriter {
public:
	ofstream vmfile;
	void init(string name) {
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.push_back('v');
		name.push_back('m');
		vmfile.open(name);
	}
	void writePush(string segment, int index) {
		vmfile << "push " << segment << " " << index << endl;
	}
	void writePop(string segment, int index) {
		vmfile << "pop " << segment << " " << index << endl;
	}
	void writeArithmetic(string command) {
		vmfile << command << endl;
	}
	void writeLabel(string label) {
		vmfile << "label " << label << endl;
	}
	void writeGoto(string label) {
		vmfile << "goto " << label << endl;
	}
	void writeIf(string label) {
		vmfile << "if-goto " << label << endl;
	}
	void writeCall(string name, int nArgs) {
		vmfile << "call " << name << " " << nArgs << endl;
	}
	void writeFunction(string name, int nLocals) {
		vmfile << "function " << name << " " << nLocals << endl;
	}
	void writeReturn() {
		vmfile << "return" << endl;
	}
};

class compilationEngine {
public:
	int nP;
	ofstream xmlfile;
	ofstream errfile;
	string nam;
	jackTokenizer j;
	vmWriter v;
	int count;
	bool stop ;
	compilationEngine(string name) {
		j.init(name);
		v.init(name);
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.push_back('x');
		name.push_back('m');
		name.push_back('l');
		xmlfile.open(name);
		name.pop_back();
		name.pop_back();
		name.pop_back();
		name.push_back('e');
		name.push_back('r');
		name.push_back('r');
		errfile.open(name);
		nP = 0;
		count = 0;
		stop = 0;
	}
	void typeMatch(string token) {
		if(stop==0)
			errfile << "ERROR: " << token << endl;
		stop = 1;
	}
	void typeMismatch(string type, string token) {
		if(stop==0)
			errfile << "ERROR: Expecting <" << type << "> but " << token << endl;
		stop = 1;
	}
	void varerror(string varName){
		if(stop==0)
			errfile<<"Declaration error: "<<varName<<" undeclared."<<endl;
		stop = 1;
	}
	void errorh(string present, string expectType) {
		if (j.tokenType(present) == expectType) {
			typeMatch(present);
		}
		else {
			typeMismatch(expectType, present);
		}
	}
	void indent() {
		for (int i = 0; i < count; i++) {
			xmlfile << "  ";
		}
	}
	void terminal() {
		indent();
		xmlfile << "<";
		xmlfile << j.tokenType() << "> ";
		if (j.tokenType() == "stringConstant")
			xmlfile << j.stringVal();
		else if (j.tokenType() == "integerConstant")
			xmlfile << j.intVal();
		else if (j.currToken == "<")
			xmlfile << "&lt;";
		else if (j.currToken == ">")
			xmlfile << "&gt;";
		else if (j.currToken == "\"")
			xmlfile << "&quot;";
		else if (j.currToken == "&")
			xmlfile << "&amp;";
		else
			xmlfile << j.currToken;
		xmlfile << " </";
		xmlfile << j.tokenType() << ">" << endl;
	}
	void nterminal(string nterm) {
		indent();
		xmlfile << "<" << nterm << "> " << endl;
		count++;
		if (nterm == "class") {
			if(compile() == "class")
				compileClass();
			else 
				errorh(j.currToken, "keyword");
		}
		else if (nterm == "classVarDec") {
			compileClassVarDec();
		}
		else if (nterm == "subroutineDec") {
			compileSubroutine();
		}
		else if (nterm == "parameterList") {
			compileParameterList();
		}
		else if (nterm == "subroutineBody") {
			terminal();
			label3:
			if (j.nextToken()) {
				if (compile() == "varDec") {
					nterminal(compile());
					goto label3;
				}
			}
			else {
				cerr;
				return;
			}
			v.writeFunction(s.currClass + "." + s.currSubroutine, s.varCount("var"));
			if (s.currSubroutineType == "constructor") {
				v.writePush("constant", s.varCount("field"));
				v.writeCall("Memory.alloc", 1);
				v.writePop("pointer", 0);
			}
			else if (s.currSubroutineType == "method") {
				v.writePush("argument", 0);
				v.writePop("pointer", 0);
			}
			if (compile() == "statements") {
				nterminal(compile());
			}
			if (j.currToken == "}") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType("}"));
			}
		}
		else if (nterm == "varDec") {
			compileVarDec();
		}
		else if (nterm == "statements") {
			compileStatements();
		}
		else if (nterm == "letStatement") {
			compileLet();
		}
		else if (nterm == "ifStatement") {
			compileIf();
		}
		else if (nterm == "whileStatement") {
			compileWhile();
		}
		else if (nterm == "doStatement") {
			compileDo();
		}
		else if (nterm == "returnStatement") {
			compileReturn();
		}
		else if (nterm == "term") {
			compileTerm();
		}
		else if (nterm == "expression") {
			compileExpression();
		}
		else if (nterm == "expressionList") {
			compileExpressionList();
		}
		count--;
		indent();
		xmlfile << "</" << nterm << ">" << endl;
	}
	string compile() {
		if (j.currToken == "class") {
			return "class";
		}
		if (j.currToken == "static" || j.currToken == "field") {
			return "classVarDec";
		}
		if (j.currToken == "constructor" || j.currToken == "function" || j.currToken == "method") {
			return "subroutineDec";
		}
		if (j.currToken == "int" || j.currToken == "char" || j.currToken == "boolean") {
			return "parameterList";
		}
		if (j.currToken == "{") {
			return "subroutineBody";
		}
		if (j.currToken == "var") {
			return "varDec";
		}
		if (j.currToken == "let" || j.currToken == "if" || j.currToken == "while" || j.currToken == "do" || j.currToken == "return") {
			return "statements";
		}
		if (j.tokenType() == "identifier") {
			return "identifier";
		}
		if(j.currToken == "void" || j.currToken == "}" || j.currToken == ")" || j.currToken == "[" || j.currToken == "]" || j.currToken == "." || j.currToken == "," || j.currToken == ";" || j.currToken == "+" || j.currToken == "*" || j.currToken == "/" || j.currToken == "&" || j.currToken == "|" || j.currToken == "<" || j.currToken == ">" || j.currToken == "="){
			return "";
		}
		return "expression";
	}
	void compileClass(){
		terminal();
		string classNam;
		if (j.nextToken()) {
			if (j.tokenType() == "identifier") {
				terminal();
				classNam = j.currToken;
				s.init(classNam);
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.currToken == "{") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType("{"));
			}
		}
		else {
			cerr;
			return;
		}
		label:
		if (j.nextToken()) {
			if (compile() == "classVarDec") {
				nterminal(compile());
				goto label;
			}
		}
		else {
			cerr;
			return;
		}
		while (compile() == "subroutineDec") {
			nterminal(compile());
			if (j.nextToken()==0) {
				cerr;
				return;
			}
		}
		if (j.currToken == "}") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType("}"));
		}
	}
	void compileClassVarDec() {
		string kind = j.currToken;
		string type;
		terminal();
		if (j.nextToken()) {
			if (compile() == "parameterList"){
				terminal();
				type = j.currToken;
			}
			else if (compile() == "identifier") {
				terminal();
				type = j.currToken;
			}
			else {
				errorh(j.currToken, "type");
			}
		}
		else {
			cerr;
			return;
		}
		label1:
		if (j.nextToken()) {
			if (j.tokenType() == "identifier") {
				terminal();
				s.define(j.currToken, type, kind);
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.currToken == ",") {
				terminal();
				goto label1;
			}
		}
		else {
			cerr;
			return;
		}
		if (j.currToken == ";") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType(";"));
		}
	}
	void compileSubroutine() {
		terminal();
		string subName, subType;
		subType = j.currToken;
		if (j.nextToken()) {
			if (compile() == "parameterList" || j.currToken == "void" || compile() == "identifier") {
				terminal();
			}
			else {
				errorh(j.currToken, "type");
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.tokenType() == "identifier") {
				terminal();
				subName = j.currToken;
				s.startSubroutine(subName, subType);
				if (subType == "method") {
					s.define("this", s.currClass, "arg");
				}
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.currToken == "(") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType("("));
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (compile() == "parameterList" || compile() == "identifier") {
				nterminal("parameterList");
			}
			else {
				if (j.currToken == ")") {
					indent();
					xmlfile << "<parameterList>" << endl;
					indent();
					xmlfile << "</parameterList>" << endl;
					terminal();
					goto l;
				}
				else {
					errorh(j.currToken, j.tokenType(")"));
				}
			}
		}
		else {
			cerr;
			return;
		}
		if (j.currToken == ")") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType(")"));
		}
		l:
		if (j.nextToken()) {
			if (compile() == "subroutineBody") {
				nterminal(compile());
			}
			else {
				errorh(j.currToken, "{");
			}
		}
		else {
			cerr;
			return;
		}
	}
	void compileParameterList() {
		string name, type;
	label2:
		if (compile() == "parameterList"){
			terminal();
			type = j.currToken;
		}
		else if (compile() == "identifier") {
			terminal();
			type = j.currToken;
		}
		else {
			errorh(j.currToken, "type");
		}
		if (j.nextToken()) {
			if (j.tokenType() == "identifier") {
				terminal();
				name = j.currToken;
			}
			else
				errorh(j.currToken, "identifier");
		}
		else {
			cerr;
			return;
		}
		s.define(name, type, "arg");
		if (j.nextToken()) {
			if (j.currToken == ",") {
				terminal();
				if (j.nextToken() == 0) {
					cerr;
					return;
				}
				goto label2;
			}
		}
		else {
			cerr;
			return;
		}
	}
	void compileVarDec() {
		terminal();
		string name, type;
		if (j.nextToken()) {
			if (compile() == "parameterList"){ 
				terminal();
				type = j.currToken;
			}
			else if(compile() == "identifier"){
				terminal();
				type = j.currToken;
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
	label4:
		if (j.nextToken()) {
			if (j.tokenType() == "identifier") {
				terminal();
				name = j.currToken;
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
		s.define(name, type, "var");
		if (j.nextToken()) {
			if (j.currToken == ",") {
				terminal();
				goto label4;
			}
		}
		else {
			cerr;
			return;
		}
		if (j.currToken == ";") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType(";"));
		}
	}
	void compileStatements() {
		while (compile() == "statements") {
			string extra = j.currToken + "Statement";
			nterminal(extra);
			if (extra == "ifStatement") {
				continue;
			}
			if (j.nextToken()==0) {
				cerr;
				return;
			}
		}
	}
	void compileDo() {
		terminal();
		if (j.nextToken()) {
			if (compile() == "identifier") {
				terminal();
				string id1, id2;
				id1 = j.currToken;
				if (j.nextToken()) {
					if (j.currToken == "(") {
						terminal();
						v.writePush("pointer", 0);
						if (j.nextToken()) {
							nterminal("expressionList");
						}
						if (j.currToken != ")") {
							errorh(j.currToken, j.tokenType(")"));
						}
						terminal();
						v.writeCall(s.currClass + "." + id1, nP+1);
						v.writePop("temp", 0);
					}
					else if (j.currToken == ".") {
						terminal();
						if (j.nextToken()) {
							if (j.tokenType() == "identifier") {
								terminal();
								id2 = j.currToken;
							}
							else {
								errorh(j.currToken, "identifier");
							}
							if (s.classScope.count(id1) || s.subroutineScope.count(id1)) {
								v.writePush(s.kindOf(id1), s.indexOf(id1));
							}
						}
						else {
							cerr;
							return;
						}
						if (j.nextToken()) {
							if (j.currToken == "(") {
								terminal();
							}
							else {
								errorh(j.currToken, j.tokenType("("));
							}
						}
						else {
							cerr;
							return;
						}
						if (j.nextToken()) {
							nterminal("expressionList");
						}
						if (j.currToken != ")") {
							errorh(j.currToken, j.tokenType(")"));
						}
						terminal();
						if (s.classScope.count(id1) || s.kindOf(id1) == "local") {
							v.writeCall(s.typeOf(id1) + "." + id2, nP + 1);
						}
						else {
							v.writeCall(id1 + "." + id2, nP);
						}
						v.writePop("temp", 0);
					}
					else {
						errorh(j.currToken, "symbol");
					}
				}
				else {
					cerr;
					return;
				}
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.currToken == ";") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType(";"));
			}
		}
		else {
			cerr;
			return;
		}
	}
	void compileLet() {
		terminal();
		string varName;
		if (j.nextToken()) {
			if (j.tokenType() == "identifier") {
				terminal();
				varName = j.currToken;
				if((s.classScope.count(varName)||s.subroutineScope.count(varName))==0)
					varerror(varName);
			}
			else {
				errorh(j.currToken, "identifier");
			}
		}
		else {
			cerr;
			return;
		}
		bool flag = 0;
		if (j.nextToken()) {
			if (j.currToken == "[") {
				terminal();
				if (j.nextToken()) {
					if (compile() == "expression" || compile() == "identifier") {
						nterminal("expression");
					}
					else {
						errorh(j.currToken, "expression");
					}
				}
				else {
					cerr;
					return;
				}
				if (j.currToken == "]") {
					terminal();
				}
				else {
					errorh(j.currToken, j.tokenType("]"));
				}
				v.writePush(s.kindOf(varName), s.indexOf(varName));
				v.writeArithmetic("add");
				if (j.nextToken()) {
					if (j.currToken == "=") {
						terminal();
					}
					else {
						errorh(j.currToken, j.tokenType("="));
					}
				}
				else {
					cerr;
					return;
				}
			}
			else if (j.currToken == "=") {
				terminal();
				flag = 1;
			}
			else {
				errorh(j.currToken, "symbol");
			}
			if (j.nextToken()) {
				if (compile() == "expression" || compile() == "identifier") {
					nterminal("expression");
				}
				else {
					errorh(j.currToken, "expression");
				}
			}
			else {
				cerr;
				return;
			}
			if(flag)
				v.writePop(s.kindOf(varName), s.indexOf(varName));
			else {
				v.writePop("temp", 0);
				v.writePop("pointer", 1);
				v.writePush("temp", 0);
				v.writePop("that", 0);
			}
			if (j.currToken == ";") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType(";"));
			}
		}
		else {
			cerr;
			return;
		}
	}
	void compileWhile() {
		terminal();
		int TlabelNum = labelNum;
		labelNum += 2;
		if (j.nextToken()) {
			if (j.currToken == "(") {
				terminal();
				if (j.nextToken()) {
					if (compile() == "expression" || compile() == "identifier") {
						v.writeLabel(s.currClass + "." + to_string(TlabelNum));
						nterminal("expression");
					}
					else {
						errorh(j.currToken, "expression");
					}
				}
				else {
					cerr;
					return;
				}
				if (j.currToken == ")") {
					terminal();
				}
				else {
					errorh(j.currToken, j.tokenType(")"));
				}
				v.writeArithmetic("not");
				v.writeIf(s.currClass + "." + to_string(TlabelNum + 1));
			}
			else {
				errorh(j.currToken, j.tokenType("("));
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.currToken == "{") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType("{"));
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (compile() == "statements") {
				nterminal(compile());
			}
		}
		else {
			cerr;
			return;
		}
		if (j.currToken == "}") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType("}"));
		}
		v.writeGoto(s.currClass + "." + to_string(TlabelNum));
		v.writeLabel(s.currClass + "." + to_string(TlabelNum + 1));
	}
	void compileReturn() {
		terminal();
		if (j.nextToken()) {
			if (compile() == "expression" || compile() == "identifier") {
				nterminal("expression");
				v.writeReturn();
			}
			else {
				v.writePush("constant", 0);
				v.writeReturn();
			}
		}
		else {
			cerr;
			return;
		}
		if (j.currToken == ";") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType(";"));
		}
	}
	void compileIf() {
		terminal();
		if (j.nextToken()) {
			if (j.currToken == "(") {
				terminal();
				if (j.nextToken()) {
					if (compile() == "expression" || compile() == "identifier") {
						nterminal("expression");
					}
					else {
						errorh(j.currToken, "expression");
					}
				}
				else {
					cerr;
					return;
				}
				if (j.currToken == ")") {
					terminal();
				}
				else {
					errorh(j.currToken, j.tokenType(")"));
				}
			}
			else {
				errorh(j.currToken, j.tokenType("("));
			}
		}
		else {
			cerr;
			return;
		}
		if (j.nextToken()) {
			if (j.currToken == "{") {
				terminal();
			}
			else {
				errorh(j.currToken, j.tokenType("{"));
			}
		}
		else {
			cerr;
			return;
		}
		int TlabelNum = labelNum;
		labelNum += 2;
		v.writeArithmetic("not");
		v.writeIf(s.currClass + "." + to_string(TlabelNum));
		if (j.nextToken()) {
			if (compile() == "statements") {
				nterminal(compile());
			}
			else {
				goto lab;
			}
		}
		else {
			cerr;
			return;
		}
	lab:
		if (j.currToken == "}") {
			terminal();
		}
		else {
			errorh(j.currToken, j.tokenType("}"));
		}
		v.writeGoto(s.currClass + "." + to_string(TlabelNum + 1));
		v.writeLabel(s.currClass + "." + to_string(TlabelNum));
		if (j.nextToken()) {
			if (j.currToken == "else") {
				terminal();
				if (j.nextToken()) {
					if (j.currToken == "{") {
						terminal();
					}
					else {
						errorh(j.currToken, j.tokenType("{"));
					}
				}
				else {
					cerr;
					return;
				}
				if (j.nextToken()) {
					if (compile() == "statements") {
						nterminal(compile());
					}
					else {
						goto lab1;
					}
				}
				else {
					cerr;
					return;
				}
			lab1:
				if (j.currToken == "}") {
					terminal();
					if (j.nextToken() == 0) {
						cerr;
						return;
					}
				}
				else {
					errorh(j.currToken, j.tokenType("}"));
				}
			}
			v.writeLabel(s.currClass + "." + to_string(TlabelNum + 1));
		}
		else {
			cerr;
			return;
		}
	}
	void compileExpression() {
		nterminal("term");
		if (j.currToken == "+" || j.currToken == "-" || j.currToken == "*" || j.currToken == "/" || j.currToken == "&" || j.currToken == "|" || j.currToken == "<" || j.currToken == ">" || j.currToken == "=") {
			terminal();
			string op = j.currToken;
			if (j.nextToken())
				nterminal("term");
			else {
				cerr;
				return;
			}
			if (op == "+")
				v.writeArithmetic("add");
			if (op == "-")
				v.writeArithmetic("sub");
			if (op == "&")
				v.writeArithmetic("and");
			if (op == "|")
				v.writeArithmetic("or");
			if (op == ">")
				v.writeArithmetic("gt");
			if (op == "<")
				v.writeArithmetic("lt");
			if (op == "=")
				v.writeArithmetic("eq");
			if (op == "*")
				v.writeCall("Math.multiply", 2);
			if (op == "/")
				v.writeCall("Math.divide", 2);
		}
	}
	void compileTerm() {
		if (j.tokenType() == "integerConstant" || j.tokenType() == "stringConstant" || j.currToken == "true" || j.currToken == "false" || j.currToken == "null" || j.currToken == "this") {
			terminal();
			if (j.tokenType() == "integerConstant")
				v.writePush("constant", j.intVal());
			if (j.currToken == "true") {
				v.writePush("constant", 0);
				v.writeArithmetic("not");
			}
			if (j.currToken == "false" || j.currToken=="null")
				v.writePush("constant", 0);
			if (j.currToken == "this")
				v.writePush("pointer", 0);
			if (j.tokenType() == "stringConstant") {
				int strlen = j.currToken.length();
				v.writePush("constant", strlen);
				v.writeCall("String.new", 1);
				for (int i = 1; i < strlen; i++) {
					v.writePush("constant", (int)j.currToken[i]);
					v.writeCall("String.appendChar", 2);
				}
			}
			if (j.nextToken() == 0) {
				cerr;
				return;
			}
		}
		else if (j.currToken == "-" || j.currToken == "~") {
			terminal();
			string op = j.currToken;
			if (j.nextToken() == 0) {
				cerr;
				return;
			}
			nterminal("term");
			if (op == "-")
				v.writeArithmetic("neg");
			else
				v.writeArithmetic("not");
		}
		else if (j.currToken == "(") {
			terminal();
			if (j.nextToken()) {
				if (compile() == "expression" || compile() == "identifier") {
					nterminal("expression");
				}
				else {
					errorh(j.currToken, "expression");
				}
			}
			else {
				cerr;
				return;
			}
			if (j.currToken != ")") {
				errorh(j.currToken, j.tokenType(")"));
			}
			terminal();
			if (j.nextToken() == 0) {
				cerr;
				return;
			}
		}
		else if (j.tokenType() == "identifier") {
			terminal();
			string varName = j.currToken;
			if (j.nextToken()) {
				if (j.currToken == "[") {
					if((s.classScope.count(varName)||s.subroutineScope.count(varName))==0)
						varerror(varName);
					terminal();
					if (j.nextToken()) {
						if (compile() == "expression" || compile() == "identifier") {
							nterminal("expression");
						}
						else {
							errorh(j.currToken, "expression");
						}
					}
					else {
						cerr;
						return;
					}
					if (j.currToken != "]") {
						errorh(j.currToken, j.tokenType("]"));
					}
					terminal();
					v.writePush(s.kindOf(varName), s.indexOf(varName));
					v.writeArithmetic("add");
					v.writePop("pointer", 1);
					v.writePush("that", 0);
					if (j.nextToken() == 0) {
						cerr;
						return;
					}
				}
				else if (j.currToken == "(") {
					terminal();
					v.writePush("pointer", 0);
					if (j.nextToken()) {
						nterminal("expressionList");
					}
					if (j.currToken != ")") {
						errorh(j.currToken, j.tokenType(")"));
					}
					terminal();
					v.writeCall(s.currClass + "." + varName, nP + 1);
					if (j.nextToken() == 0) {
						cerr;
						return;
					}
				}
				else if (j.currToken == ".") {
					terminal();
					string id2;
					if (j.nextToken()) {
						if (j.tokenType() == "identifier") {
							terminal();
							id2 = j.currToken;
						}
						else {
							errorh(j.currToken, "identifier");
						}
					}
					else {
						cerr;
						return;
					}
					if (s.classScope.count(varName) || s.subroutineScope.count(varName)) {
						v.writePush(s.kindOf(varName), s.indexOf(varName));
					}
					if (j.nextToken()) {
						if (j.currToken == "(") {
							terminal();
						}
						else {
							errorh(j.currToken, j.tokenType("("));
						}
					}
					else {
						cerr;
						return;
					}
					if (j.nextToken()) {
						nterminal("expressionList");
					}
					if (j.currToken != ")") {
						errorh(j.currToken, j.tokenType(")"));
					}
					terminal();
					if (s.classScope.count(varName) || s.kindOf(varName) == "local") {
						v.writeCall(s.typeOf(varName) + "." + id2, nP + 1);
					}
					else {
						v.writeCall(varName + "." + id2, nP);
					}
					if (j.nextToken() == 0) {
						cerr;
						return;
					}
				}
				else {
					if((s.classScope.count(varName)||s.subroutineScope.count(varName))==0)
						varerror(varName);
					v.writePush(s.kindOf(varName), s.indexOf(varName));
				}
			}
			else {
				cerr;
				return;
			}
		}
		else {
			errorh(j.currToken, "term");
		}
	}
	void compileExpressionList() {
		nP = 0;
		if (compile() == "expression" || compile() == "identifier") {
		la:
			nterminal("expression");
			nP++;
			if (j.currToken == ",") {
				terminal();
				if (j.nextToken())
					goto la;
				else {
					cerr;
					return;
				}
			}
		}
	}
};

int main(int argc, char* argv[]) {	
	for (int i=2;i<argc;i++) {
		labelNum = 0;
		string path = argv[i];
		compilationEngine c(path);
		if (c.j.nextToken() == 0) {
			cerr;
			return 0;
		}
		c.nterminal("class");
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
