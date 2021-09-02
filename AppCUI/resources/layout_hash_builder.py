keys = {
	"x"	:"x",
        "y"	:"y",
	"l"	:"left",
	"left"	:"left",
	"r"	:"right",
	"right"	:"right",
	"t"	:"top",
	"top"	:"top",
	"b"	:"bottom",
	"bottom":"bottom",
	"w"	:"width",
	"width"	:"width",
	"h"	:"height",
	"height":"height",
	"a"	:"align",
	"align"	:"align",
	"d"	:"dock",
	"dock"	:"dock"
}

align_values = ["tl","topleft","lt","lefttop",
		"t","top","tc","topcenter",
		"tr","topright","righttop",
		"r","right","rc","rightcenter",
		"br","bottomright","rightbottom",
		"b","bottom","bc","bottomcenter",		
		"bl","bottomright","rightbottom"]

def ComputeHash(s):
	s = s.upper()
	h = 0
	for k in s:
		idx = (ord(k)-ord('A'))+1
		#h = h * 2 + 
		h += idx
	return h

def ComputeHashes(d_list):
	d = {}	
	for k in d_list:
		h = ComputeHash(k)
		if not k in d:
			d[h] = d_list[k]
		if d[h]!=d_list[k]:
			print("Colission: key:'"+k+"' mapped to '"+d_list[k]+"' has the same hash as keys mapped to '"+d[k]+"' !")
			return None
	return d

def CreateKeys():
	res = ComputeHashes(keys)
	if not res: return
	d = {}
	for k in keys:
		d[keys[k]] = 1
	s = "constexpr unsigned short LAYOUT_KEY_NONE = 0;\n"
	v = 1;
	for k in d:
		s += "constexpr unsigned short LAYOUT_KEY_"+k.upper()+" = 0x%04X;\n"%(v);
		v *= 2
	s += "\n"
	s += "constexpr unsignd short _layout_translate_map_["+str(max(res)+1)+"] = "
	for h in range(0,max(res)+1):
		if h in res:
			s += "LAYOUT_KEY_"+res[h].upper()+","
		else:
			s += "LAYOUT_KEY_NONE,"
	s = s[:-1] + "};\n"
	s += "\n"; 
	s += "inline unsigned short LayoutKeyToFlag(unsigned int hash) {\n";
	s += "	if (hash>="+str(max(res)+1)+") return LAYOUT_KEY_NONE;\n";
	s += "	retunr _layout_translate_map_[hash];\n"
	s += "};\n"
	return s

s =  "\n//========================================="
s += "\n// THIS CODE WAS AUTOMATICALLY GENERATED !"
s += "\n//========================================="
s += "\n"
s += "\n"+CreateKeys()
s += "\n"
s += "\n//========================================="
s += "\n// END OF AUTOMATICALLY GENERATED CODE"
s += "\n//========================================="
s += "\n"

print(s)

	


