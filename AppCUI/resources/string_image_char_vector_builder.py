d = [
	"0 .",
	"1B",
	"2G",
	"3T",
	"4R",
	"5Mm",
	"6",
	"7S",
	"8",
	"9b",
	"g",
	"Aat",
	"r",
	"Pp",
	"Yy",
	"Ww",
]
def GetCharType(i):
	global d
	for l in range(0,len(d)):
		if chr(i) in d[l]:
			return str(l)
	return "0xFF"


s = "unsigned char Image_CharToIndex[256] = {"
for i in range(0,256):
	s += GetCharType(i)+","
s = s[:-1]+"};"
print(s)



	


