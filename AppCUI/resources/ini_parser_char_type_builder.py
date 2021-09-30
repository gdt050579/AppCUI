
def GetCharType(i):
	if i>=ord('A') and i<=ord('Z'): return "CHAR_TYPE_WORD"
	if i>=ord('a') and i<=ord('z'): return "CHAR_TYPE_WORD"
	if i>=ord('0') and i<=ord('9'): return "CHAR_TYPE_NUMBER"
	if (i==ord('_')) or (i==ord('.')): return "CHAR_TYPE_WORD"
	if (i==32) or (i==ord('\t')): return "CHAR_TYPE_SPACE"
	if (i==ord('\n')) or (i==ord('\r')): return "CHAR_TYPE_NEW_LINE"
	if (i==ord('"')) or (i==ord('\'')): return "CHAR_TYPE_STRING"
	if (i==ord(';')) or (i==ord('#')): return "CHAR_TYPE_COMMENT"
	if (i==ord('=')): return "CHAR_TYPE_EQ"
	if (i==ord('[')): return "CHAR_TYPE_SECTION_START"
	if (i==ord(']')): return "CHAR_TYPE_SECTION_END"
	if (i==ord(',')): return "CHAR_TYPE_COMMA"
	return "CHAR_TYPE_OTHER"


s = "unsigned char __char_type__[256] = {"
for i in range(0,256):
	s += GetCharType(i)+","
s = s[:-1]+"};"
print(s)



	


