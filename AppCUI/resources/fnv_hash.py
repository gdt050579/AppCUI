def ComputeFNVHash(s):
	hash = 0xcbf29ce484222325
	for c in s.lower():
		hash = hash ^ (ord(c) ^ 0xFF)
		hash = hash & 0xFFFFFFFFFFFFFFFF
		hash = hash * 0x00000100000001B3
		hash = hash & 0xFFFFFFFFFFFFFFFF
	return hash

m = {
	"n":"HASH_ID_NAME",
	"name":"HASH_ID_NAME",
	"a":"HASH_ID_ALIGN",
	"align":"HASH_ID_ALIGN",
	"alignament":"HASH_ID_ALIGNAMENT",
	"w":"HASH_ID_WIDTH",
	"width":"HASH_ID_WIDTH"
}

def CreateHashes(m):
	l = len(m)
	while True:
		d = {}
		for k in m:
			newHash = ComputeFNVHash(k) % l
			if newHash in d:
				break
			d[newHash] = 1
		if len(d)==len(m):
			#found one hash
			print(l)
			break
		else:
			l = l + 1


CreateHashes(m)
#print(hex(ComputeFNVHash("abc")))