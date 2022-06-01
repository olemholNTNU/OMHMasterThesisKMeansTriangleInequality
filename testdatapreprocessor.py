import sys

textelementdict = {}
uniqTextId = 0
uniqId = "uniqTextIdCounter"

answerList = {}

datainputfile = "KDDTrain+_20Percent.txt"

if len(sys.argv) >= 2:
	datainputfile = sys.argv[1]

with open(datainputfile , 'r') as source:
	data = source.readlines()
	for item in data:
		splitItem = item.rstrip().split(',')
		currentitemtype = splitItem[1]
		if currentitemtype not in answerList:
			answerList[currentitemtype] = []
		with open(datainputfile.split('.')[0] + "processedSet"+ currentitemtype + ".txt", 'a') as f:
			del splitItem[1]
			del splitItem[-1]
#			print(splitItem[40] == "normal")
			answerList[currentitemtype].append("1" if "normal" == splitItem.pop(40) else "0")
			outdata = ""
			for index, a in enumerate(splitItem):
				if not a.isnumeric():
					if "." in a:
						try:
							a = float(a)
						except ValueError:
							print("Not a float")
						if len(sys.argv) < 3 or sys.argv[2] != "-f":
							a = int(a*100)
						outdata += str(a)
					else:
						if str(index) not in textelementdict:
							textelementdict[str(index)] = {uniqId: 1, a: 0}
						else:
							if a not in textelementdict[str(index)]:
								textelementdict[str(index)][a] = textelementdict[str(index)][uniqId]
								textelementdict[str(index)][uniqId] += 1
						if len(sys.argv) < 4 or sys.argv[3] != "-ns":
							outdata += str(textelementdict[str(index)][a])
				else:
					outdata += a
				outdata += " "
			outdata = outdata[:-1] + "\n"
			# Do stuff


			f.write(outdata)


	with open(datainputfile.split('.')[0] + "TextElementDict.txt", 'w') as o:
		output = ""
		for key, value in textelementdict.items():
			output += "{upperkey}:\n".format(upperkey=key)
			for internalkey, internalvalue in value.items():
				output += "\t{keyitem} - {valueitem}\n".format(keyitem=internalkey, valueitem=internalvalue)
		o.write(output)


	for itemtype, answerarray in answerList.items():
		with open(datainputfile.split('.')[0] + "processedSet" + itemtype + ".txtAnswerList" + ".txt", 'w') as answer:
			outtext = ""
			for aitem in answerarray:
				outtext += aitem + "\n"
			answer.write(outtext)
