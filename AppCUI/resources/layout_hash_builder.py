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

align_values_reversed = {
	"TopLeft":"tl,lt,topleft,lefttop",
	"Top":"t,top",
	"TopRight":"tr,rt,topright,righttop",
	"Right":"r,right",
	"BotomRight":"br,rb,bottomright,rightbottom",
	"Bottom":"b,bottom",
	"BotomLeft":"bl,lb,bottomleft,leftbottom",
	"Left":"l,left",
	"Center":"c,center",
}

def GenerateAlignValue():
	global align_values_reversed
	d = {}
	for k in align_values_reversed:
		for v in align_values_reversed[k].split(","):
			d[v] = k
	return d;

def ComputeHash(s):
	s = s.upper()
	h = 0
	index = 0
	for k in s:
		ch_id = (ord(k)-ord('A'))+1
		#h = h * 2 + ch_id
		h = h + ch_id + index
		index+=2
		#h += ch_id
	return h

def ComputeHashes(d_list):
	d = {}	
	for k in d_list:
		h = ComputeHash(k)
		if not h in d:
			d[h] = d_list[k]
		if d[h]!=d_list[k]:
			print("Colission: key:'"+k+"' mapped to '"+d_list[k]+"' has the same hash as keys mapped to '"+d[h]+"' !")
			return None
	return d

def CreateKeys():
	res = ComputeHashes(keys)
	if not res: return
	d = {}
	for k in keys:
		d[keys[k]] = 1
	s = "constexpr unsigned char LAYOUT_KEY_NONE = 0;\n"
	v = 1;
	idx = 1
	for k in d:
		s += "constexpr unsigned short LAYOUT_KEY_"+k.upper()+" = %d;\n"%(idx);
		s += "constexpr unsigned short LAYOUT_FLAG_"+k.upper()+" = 0x%04X;\n"%(v);
		v *= 2
		idx+=1
	s += "\n"
	s += "constexpr unsigned char _layout_translate_map_["+str(max(res)+1)+"] = {"
	for h in range(0,max(res)+1):
		if h in res:
			s += "LAYOUT_KEY_"+res[h].upper()+","
		else:
			s += "LAYOUT_KEY_NONE,"
	s = s[:-1] + "};\n"
	s += "\n"; 
	s += "inline unsigned char HashToLayoutKey(unsigned int hash) {\n";
	s += "	if (hash>="+str(max(res)+1)+") return LAYOUT_KEY_NONE;\n";
	s += "	return _layout_translate_map_[hash];\n"
	s += "};\n"
	return s

def CreateAlignValues():
	av = GenerateAlignValue()
	res = ComputeHashes(av)
	if not res: return
	s = ""
	#s += "/* HASH VALUES FOR ALIGN:\n"
	#for h in res:
	#	s += "    %s => %d\n"%(res[h],h)
	#s += "*/\n"
	s += "constexpr unsigned char _align_translate_map_["+str(max(res)+1)+"] = {"
	for h in range(0,max(res)+1):
		if h in res:
			s += "Alignament."+res[h]+","
		else:
			s += "0xFF,"
	s = s[:-1] + "};\n"
	s += "\n"; 
	s += "inline bool HashToAlignament(unsigned int hash, Alignament & align) {\n";
	s += "	if (hash>="+str(max(res)+1)+") return false;\n";
	s += "	auto ch = _align_translate_map_[hash];\n";
	s += "	if (ch == 0xFF) return false;\n";
	s += "	align = static_cast<Alignament>(ch);\n";
	s += "	return true;\n"
	s += "};\n"
	return s

s =  "\n//========================================="
s += "\n// THIS CODE WAS AUTOMATICALLY GENERATED !"
s += "\n//========================================="
s += "\n"
s += "\n"+CreateKeys()
s += "\n"
s += "\n"+CreateAlignValues()
s += "\n"
s += "\n//========================================="
s += "\n// END OF AUTOMATICALLY GENERATED CODE"
s += "\n//========================================="
s += "\n"

print(s)

	


