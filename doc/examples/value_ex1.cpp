#include <grace/value.h>

void foo (void)
{
    value v;
    v.type ("phoneBook");
    
    v["john"]["firstName"] = "John";
    v[-1]("type") = "friend";
    v[-1]["lastName"] = "Smith";
    v[-1]["phoneNumber"] = "212-555-1234";
    v[-1]["userId"] = 15;
    
    v["pete"]["firstName"] = "Peter";
    v[-1]("type") = "coworker";
    v[-1]["lastName"] = "O'Connor";
    v[-1]["phoneNumber"] = "212-555-4321";
    v[-1]["userId"] = 43;
}
