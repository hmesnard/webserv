#include "Server.hpp"

Virtual_server::Virtual_server(Rules const &serv_rules)
{
	rule_set = serv_rules;
}


Rules &Virtual_server::get_rules(void)
{
	return (rule_set);
}
