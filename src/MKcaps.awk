
#  This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for
#  details. If they are missing then this copy is in violation of  
#  the copyright conditions. 

BEGIN		{
			printf "\t%-32s\t%s\t\t%s\n", "Variable", "Capname", "Description" > "boolcaps"
			print  ""  > "bools"
			print  "\tBoolean capabilities:" > "bools"
			print  "" > "bool"
			print  ""  > "numcaps"
			print  "\tNumerical capabilities:" > "numcaps"
			print  "" > "numcaps"
			print  ""  > "strcaps"
			print  "\tString capabilities:" > "strcaps"
			print  "" > "strcaps"
		}

$3 == "bool"	{
			printf "\t%-32s\t%s\n", $1, $2 > "boolcaps"
		}

$3 == "num"	{
			printf "\t%-32s\t%s\n", $1, $2 > "numcaps"
		}

$3 == "str"	{
			printf "\t%-32s\t%s\n", $1, $2 > "strcaps"
		}

