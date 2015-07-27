#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <deque>
#include <cctype>
#include <gmpxx.h>

// note to jt: I used to use std::shared_ptr in here
// but I found I could shave 150ms off my run time by leaking memory
// since nothing is freed until the program exits anyway.
// it's your fault for the perverse incentives.

using namespace std;

typedef mpz_class val_type;

ostream& operator<<(ostream& os, const mpz_class &bigint)
{
	return os << bigint.get_str();
}

typedef unordered_map<string, val_type> varmap;
varmap vars;

bool trace_tree = false;
bool trace_exec = false;

class UnexpectedToken : public exception
{
	string m_what;
	
public:
	UnexpectedToken(string expected, string found)
		: m_what("Unexpected token: " + found + "; expected " + expected)
	{}
	const char *what() const noexcept { return m_what.c_str(); }
};

class UninitializedVariable : public exception
{
	string m_what;
	
public:
	UninitializedVariable(string name)
		: m_what("Uninitialized variable: " + name)
	{}
	const char *what() const noexcept { return m_what.c_str(); }
};


class Tokenizer
{
	deque<string> lookahead;
	
	string read()
	{
		char c = cin.get();
		while(c != EOF && isspace(c))
			c = cin.get();
		if (c == EOF)
			return string();
			
		string token(1, c);
		if (isalpha(c))
		{
			while(isalnum(cin.peek()))
				token += cin.get();
		}
		else if (isdigit(c))
		{
			while(isdigit(cin.peek()))
				token += cin.get();
		}
		else if (c == '+')
		{
			if (cin.peek() == '=')
				token += cin.get();
		}
		else if (c == '-')
		{
			if (cin.peek() == '=')
				token += cin.get();
		}
		return token;
	}

public:
	string next()
	{
		string token;
		if (!lookahead.empty()) {
			token = lookahead.front();
			lookahead.pop_front();
		} else {
			token = read();
		}
//		cerr << '\t' << token << endl;
		return token;
	}
	
	void eat(const string &expected)
	{
		string found = next();
		if (found != expected)
			throw UnexpectedToken(expected, found);
	}

	
	string peek1()
	{
		if (lookahead.empty()) {
			lookahead.push_back(read());
		}
		return lookahead.front();
	}
	
	string peek2()
	{
		while(lookahead.size() < 2) {
			lookahead.push_back(read());
		}
		return lookahead[1];
	}
};

Tokenizer tokenizer;

class ParseError : public exception
{
	string m_what;
	
public:
	ParseError(string what)
		: m_what(what + ": near " + tokenizer.peek1() + " " + tokenizer.peek2())
	{}
	const char *what() const noexcept { return m_what.c_str(); }
};

class Node
{
public:
	virtual void inspect() = 0;
};

class Value : public Node
{
public:
	virtual val_type val() = 0;
	static Value *parse();
};

class Variable : public Value
{
public:
	string name;
	static Variable *parse()
	{
		Variable *v = new Variable;
		v->name = tokenizer.next();
		return v;
	}
	
	val_type val()
	{
		varmap::iterator it = vars.find(name);
		if (it != vars.end())
			return it->second;
		throw UninitializedVariable(name);
	}
	
	void assign(val_type val)
	{
		varmap::iterator it = vars.find(name);
		if (it != vars.end())
			it->second = val;
		else
			vars.insert(varmap::value_type(name, val));
	}
	
	void inspect() { cerr << "var " << name; }
};

class Integer : public Value
{
public:
	val_type value;
	static Integer *parse()
	{
		Integer *i = new Integer;
		//i->value = strtoll(tokenizer.next().c_str(), NULL, 10);
		i->value = tokenizer.next();
		return i;
	}
	
	val_type val()
	{
		return value;
	}
	
	void inspect() { cerr << "int " << value; }
};

/* static */ Value *Value::parse() 
{
	string token = tokenizer.peek1();
	if (token.empty())
		throw ParseError("invalid Value");
	if (isdigit(token[0]))
		return Integer::parse();
	if (isalpha(token[0]))
		return Variable::parse();	
	throw ParseError("invalid Value");
}

class Runnable : public Node
{
public:
	virtual void run() = 0;
};

class Statement : public Runnable
{
public:
	static Statement *parse();
};

class Statements : public Node
{
public:
	static void parse(vector<Statement *> &statements)
	{
		for(;;)
		{
			Statement *s(Statement::parse());
			if (s)
				statements.push_back(s);
			else
				break;
				
			if (tokenizer.peek1() == ";")
				tokenizer.eat(";");
			else
				break;
		}
	}
	
	static void inspect(vector<Statement *> &statements)
	{
		for(size_t i = 0; i < statements.size(); ++i)
		{
			cerr << i << ". ";
			statements[i]->inspect();
			cerr << endl;
		}
	}
};

class IfZero : public Statement
{
public:
	Value *value;
	vector<Statement *> statements;

	static IfZero *parse();
	
	void run()
	{
		if (trace_exec)
			cerr << value->val() << "?" << endl;
		if (value->val() == 0)
		{
			for(size_t i = 0; i < statements.size(); ++i)
				statements[i]->run();
		}
	}
	
	void inspect()
	{
		cerr << "ifzero ";
		value->inspect();
		cerr << "\n{\n";
		Statements::inspect(statements);
		cerr << "}";
	}
};

class Assignment : public Statement
{
public:
	Variable *variable;
	Value *value;
	
	static Assignment *parse()
	{
		Assignment *a = new Assignment;
		a->variable = Variable::parse();
		tokenizer.eat("=");
		a->value = Value::parse();
		return a;
	}	
	
	void run()
	{
		if (trace_exec)
			cerr << variable->name << " = " << value->val() << endl;
		variable->assign(value->val());
	}
	
	void inspect()
	{
		cerr << "assign ";
		variable->inspect();
		cerr << ", ";
		value->inspect();
	}
};

class Addition : public Statement
{
public:
	Variable *variable;
	Value *value;
	
	static Addition *parse()
	{
		Addition *a = new Addition;
		a->variable = Variable::parse();
		tokenizer.eat("+=");
		a->value = Value::parse();
		return a;
	}
	
	void run()
	{
		if (trace_exec)
			cerr << variable->name << " += " << value->val() << endl;
		variable->assign( variable->val() + value->val() );
	}
	
	void inspect()
	{
		cerr << "add ";
		variable->inspect();
		cerr << ", ";
		value->inspect();
	}
};

class Subtraction : public Statement
{
public:
	Variable *variable;
	Value *value;
	
	static Subtraction *parse()
	{
		Subtraction *s = new Subtraction;
		s->variable = Variable::parse();
		tokenizer.eat("-=");
		s->value = Value::parse();
		return s;
	}
	
	void run()
	{
		if (trace_exec)
			cerr << variable->name << " -= " << value->val() << endl;
		variable->assign( variable->val() - value->val() );
	}
	
	void inspect()
	{
		cerr << "sub ";
		variable->inspect();
		cerr << ", ";
		value->inspect();
	}
};

class Println : public Statement
{
public:
	Value *value;
	
	static Println *parse()
	{
		tokenizer.eat("!");
		Println *p = new Println;
		p->value = Value::parse();
		return p;
	}
	
	void run()
	{
		cout << value->val() << "\n";
	}
	
	void inspect()
	{
		cerr << "println ";
		value->inspect();
	}
};

/* static */ Statement *Statement::parse()
{
	if (tokenizer.peek1() == "?")
		return IfZero::parse();
	if (tokenizer.peek1() == "!")
		return Println::parse();
	if (tokenizer.peek2() == "=")
		return Assignment::parse();
	if (tokenizer.peek2() == "+=")
		return Addition::parse();
	if (tokenizer.peek2() == "-=")
		return Subtraction::parse();
	throw ParseError("incorrect statement");
}

/* static */ IfZero *IfZero::parse()
{
	tokenizer.eat("?");
	IfZero *i = new IfZero;
	i->value = Value::parse();
	tokenizer.eat("{");
	Statements::parse(i->statements);
	tokenizer.eat("}");
	return i;
}


class Program : public Runnable
{
public:
	vector<Statement *> statements;
	
	static Program *parse()
	{
		Program *p = new Program;
		Statements::parse(p->statements);
		return p;
	}
	
	void run()
	{
		for(size_t i = 0; i < statements.size(); ++i)
			statements[i]->run();
	}
	
	void inspect()
	{
		Statements::inspect(statements);
	}
};

int main(int argc, char **argv)
{
	if (argc >= 2)
	{
	 	if (argv[1][0] == 't')
			trace_tree = true; 
		else if (argv[1][0] == 'e')
			trace_exec = true;
	}
	
	try
	{
		Program *prog = Program::parse();
		if (trace_tree)
			prog->inspect();
		prog->run();
	}
	catch(exception &e)
	{
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}