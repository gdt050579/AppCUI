import os,sys

data = {}

template = r"""
namespace ${NAMESPACE} {
	enum class Value: uint8
	{
		${VALUES}
		Invalid = 0xFF
	};
	static constexpr Value Values[${DEVIDER}] = {
		${TABLE_VALUES}
	};
	static constexpr uint64 Hashes[${DEVIDER}] = {
		${TABLE_HASHES}
	};
	inline Value HashToValue(uint64 hash) {
		const auto entry = hash % ${DEVIDER};
		if (Hashes[entry]!=hash)
			return Value::Invalid;
		return Values[entry];
	}
};
"""

def ComputeFNVHash(s):
	hash = 0xcbf29ce484222325
	for c in s.lower():
		hash = hash ^ (ord(c) ^ 0xFF)
		hash = hash & 0xFFFFFFFFFFFFFFFF
		hash = hash * 0x00000100000001B3
		hash = hash & 0xFFFFFFFFFFFFFFFF
	return hash

def Error(msg):
	print("[ERROR] -> ",msg)

def Info(msg):
	print("[INFO ] -> ",msg)

def LoadIni(fname):
	global data
	section = ""
	data["list"] = {}
	data["general"] = {}
	for line in open(fname,"rt"):
		line = line.strip()
		#skip empty lines
		if len(line)==0:
			continue
		# skip comments
		if line.startswith("#") or line.startswith(";"):
			continue
		# check if section
		if line.startswith("["):
			if line == "[general]":
				section = "general"
				continue
			elif line == "[list]":
				section = "list"
				continue
			else:
				Error("Invalid section `"+line+"` in file `"+fname+"`. Allowed sections are [general] and [list] !");
				return False
		# else we should have a key = value pair
		if not "=" in line:
			Error("Expectin a key = value pair in `"+line+"`")
			return False
		k = line.split("=",1)[0].lower().strip()
		v = line.split("=",1)[1].strip()
		if section == "":
			Error("No section defined for `"+line+"`")
			return False
		data[section][k] = v

	#generic checks
	if not "namespace" in data["general"]:
		Error("Namespace field must be defined for [general] section")
		return False
	if len(data["list"])==0:
		Error("No values added to [list] section")
		return False
	return True		

def ComputeHashDevider(m):
	l = len(m)
	while True:
		d = {}
		for k in m:
			newHash = ComputeFNVHash(k) % l
			if newHash in d:
				break
			d[newHash] = 1
		if len(d)==len(m):
			return l
		else:
			l = l + 1

def BuildCode(devider):
	global data
	global template
	values = ""
	idx = 0
	table_values = ["Invalid"]*devider
	table_hashes = [0]*devider
	results = {}
	for k in data["list"]:
		if data["list"][k] not in results:
			values = values + data["list"][k]+" = "+str(idx)+",\n\t\t"
			results[data["list"][k]] = idx
			idx+=1
		hash = ComputeFNVHash(k)
		d_idx = hash % devider
		table_values[d_idx] = data["list"][k]
		table_hashes[d_idx] = hash
	s_table_values = ""
	for k in table_values:
		s_table_values+="Values::"+k+","
	s_table_hashes = ""
	for k in table_hashes:
		s_table_hashes+="0x%X,"%k
	s = template
	s = s.replace(r"${NAMESPACE}",data["general"]["namespace"])
	s = s.replace(r"${DEVIDER}",str(devider))
	s = s.replace(r"${VALUES}",values)
	s = s.replace(r"${TABLE_VALUES}",s_table_values)
	s = s.replace(r"${TABLE_HASHES}",s_table_hashes)
	return s

def main():
	global data
	if len(sys.argv)!=2:
		print("Usage: ini_parser_builde.py <ini_file>")
		return
	if LoadIni(sys.argv[1])==False:
		return
	devider = ComputeHashDevider(data["list"])
	Info("Devider  : "+str(devider))
	Info("Elements : "+str(len(data["list"])))
	Info("Namespace: "+data["general"]["namespace"])
	code = BuildCode(devider)
	try:
		open(sys.argv[1]+".cpp","wt").write(code)
		Info("Save file: OK")
	except:
		Error("Fail to create file: "+sys.argv[1]+".cpp")
		return

main()