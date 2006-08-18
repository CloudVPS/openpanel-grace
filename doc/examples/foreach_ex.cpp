value v;
v["category"] = "foods";
v["code"] = "CABB0001";
v["description" = "Cabbage";
v["price"] = 14.95;

foreach (node, v)
{
	fout.printf ("%s = ", node.id().str());
	
	caseselector (node.type())
	{
		incaseof (t_integer) :
			fout.printf ("%i", node.ival());
			break;
		
		incaseof (t_double) :
			fout.printf ("%.2f", node.dval());
			break;
		
		defaultcase:
			fout.printf ("\"%S\"", node.cval());
			break;
	}
	fout.printf ("\n");
}
