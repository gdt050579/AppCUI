
def GetCharType(i):
	if (i==32) or (i==ord('\t')): return "CHAR_TYPE_SPACE"
	if (i==ord('\n')) or (i==ord('\r')): return "CHAR_TYPE_SPACE"
	if (i==ord(',')) or (i==ord(';')): return "CHAR_TYPE_SEPARATOR"
	if (i==ord(':')) or (i==ord('=')): return "CHAR_TYPE_EQ"
	return "CHAR_TYPE_OTHER"


s = "unsigned char __char_type__[<XXX>] = {"
last = 0
l = []
for i in range(0,256):
	res = GetCharType(i)
	if res!="CHAR_TYPE_OTHER":
		last = i
	l += [res]
s = s.replace("<XXX>",str(last+1))
for i in l[:last+1]:
	s += i+","
s = s[:-1]+"};"
print(s)



	


