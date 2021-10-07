
def GetCharValue(i):
	if i>=ord('0') and i<=ord('9'): return i-ord('0')
	if i>=ord('A') and i<=ord('F'): return i-ord('A')+10
	if i>=ord('a') and i<=ord('z'): return i-ord('a')+10
	return 255


s = "unsigned char __base_translation__[256] = {"
for i in range(0,256):
	s += str(GetCharValue(i))+","
s = s[:-1]+"};"
print(s)



	


