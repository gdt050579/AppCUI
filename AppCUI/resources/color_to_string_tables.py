colors = ["Black", "DarkBlue", "DarkGreen", "Teal", "DarkRed", "Magenta", "Olive", "Silver",      "Gray",
    "Blue",  "Green",    "Aqua",      "Red",  "Pink",    "Yellow",  "White", "Transparent"];
def c_hash(col):
	s = 0
	for ch in col.lower():
		s = (s<<1) ^ (ord(ch)-ord('a'))
	return s

def check_div(value):
	d = {}
	for col in colors:
		v = c_hash(col)%value
		if v in d:
			#print("Colition for:",col);
			return False
		d[v] = True
	return True


for i in range(17,100):
	if check_div(i)==True:
		#print("Found : ",i);
		d = {}
		col_index = 0
		for col in colors:
			#print(col,"=>",c_hash(col)%i)
			d[c_hash(col)%i] = col_index
			col_index+=1
		#build the string
		s = "#define HASH_DEVIDER "+str(i)+"\n"
		s += "uint8 color_indexes[HASH_DEVIDER] = {"
		for k in range(0,i):
			if k in d:
				s+=str(d[k])+","
			else:
				s+="0xFF,"
		s = s[:-1]+"};\n"
		print(s)	
		break;