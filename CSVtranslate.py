from csv import reader

#start configurable parameters
cfg_huffman_0_max_time = 5e-07
cfg_huffman_1_max_time = 1e-05
cfg_morse_unit_time_s = .5
cfg_file_path_and_name = 'digital.csv'
#end configurable parameters

#start constant parameters
const_amountbitsmorse = 16
const_amountbitshuffman = 8
const_on_state = 1
const_off_state = 0
#end constant parameters

#start list declarations
csv_times = []
csv_states = []
diftimes = []
morsestring = []
huffmanstring = []
huffman = []
morse = []
#end list declarations

#start dictionary definitions
dic_morse = [[0x8000, 'E'],
  [0xA000, 'I'],
  [0xA800, 'S'],
  [0xAA00, 'H'],
  [0xAA80, '5'],
  [0xAAC0, '4'],
  [0xAB00, 'V'],
  [0xAB60, '3'],
  [0xAC00, 'U'],
  [0xAD00, 'F'],
  [0xADB0, '2'],
  [0xB000, 'A'],
  [0xB400, 'R'],
  [0xB500, 'L'],
  [0xB600, 'W'],
  [0xB680, 'P'],
  [0xB6C0, 'J'],
  [0xB6D8, '1'],
  [0xC000, 'T'],
  [0xD000, 'N'],
  [0xD400, 'D'],
  [0xD500, 'B'],
  [0xD540, '6'],
  [0xD580, 'X'],
  [0xD600, 'K'],
  [0xD680, 'C'],
  [0xD6C0, 'Y'],
  [0xD800, 'M'],
  [0xDA00, 'G'],
  [0xDA80, 'Z'],
  [0xDAA0, '7'],
  [0xDAC0, 'Q'],
  [0xDB00, 'O'],
  [0xDB50, '8'],
  [0xDB68, '9'],
  [0xDB6C, '0']]

dic_huffman = [[0x00, '5'],
  [0x08, '6'],
  [0x10, '1'],
  [0x20, '0'],
  [0x30, '3'],
  [0x40, 'V'],
  [0x41, 'Z'],
  [0x42, 'W'],
  [0x44, 'M'],
  [0x48, 'A'],
  [0x50, '2'],
  [0x60, 'E'],
  [0x68, 'N'],
  [0x70, '4'],
  [0x80, ' '],
  [0x84, 'P'],
  [0x88, '9'],
  [0x90, '8'],
  [0x98, 'T'],
  [0xA0, '7'],
  [0xA8, 'O'],
  [0xB0, 'G'],
  [0xB4, 'F'],
  [0xB8, 'U'],
  [0xC0, 'Y'],
  [0xC2, 'K'],
  [0xC4, 'H'],
  [0xC8, 'I'],
  [0xD0, 'R'],
  [0xD8, 'C'],
  [0xE0, 'J'],
  [0xE1, 'Q'],
  [0xE2, 'X'],
  [0xE4, 'B'],
  [0xE8, 'L'],
  [0xF0, 'S'],
  [0xF8, 'D']]
#end dictionary definitions

#generic binary search to look in the ordered dictionaries for a match
def binary_search(arr, x):
    low = 0
    high = len(arr) - 1
    
    while low <= high:
        mid = (low + high) // 2
        if arr[mid][0] == x:
            return mid
        elif arr[mid][0] < x:
            low = mid + 1
        else:
            high = mid - 1
    
    return -1

#translator from times captured to bits to get a final 8 bits outcome based on Huffman parameters
def huffmanchar(bits):
    outcome = 0
    for i in range(0,len(bits)):
        if(bits[i] > cfg_huffman_0_max_time):
            outcome |= (1 << ((const_amountbitshuffman-1)-i))
    searchoutcome = binary_search(dic_huffman,outcome)
    if(searchoutcome != -1):
        huffmanstring.append(dic_huffman[searchoutcome][1])
        
#translator from times captured to bits to get a final 16 bits outcome based on Morse parameters
def morsechar(bits):
    outcome = 0
    counter = 0
    for i in range(0,len(bits)):
        if(bits[i] > (cfg_morse_unit_time_s*2)):
            outcome |= (1 << ((const_amountbitsmorse-1)-counter))
            counter += 1
            outcome |= (1 << ((const_amountbitsmorse-1)-counter))
            counter += 2
        else:
            outcome |= (1 << ((const_amountbitsmorse-1)-counter))
            counter += 2
    searchoutcome = binary_search(dic_morse,outcome)
    if(searchoutcome != -1):
        morsestring.append(dic_morse[searchoutcome][1])            

#open CSV file as "read only" to get changes of state on channel 0
with open(cfg_file_path_and_name,'r') as read_obj:
    csv_reader= reader(read_obj)
    header = next(csv_reader)
    if header != None:
        for row in csv_reader:
            csv_times.append(float(row[0]))
            csv_states.append(int(row[1]))

#Using the data from the CSV we get the difference between times to know how long each pulse lasted
for i in range(0,len(csv_times)-1):
    diftimes.append(csv_times[i+1]-csv_times[i])

#We go through all the pulses to translate them to their proper meaning
for i in range(0,len(diftimes)):
    #Huffman data
    if(diftimes[i] < cfg_huffman_1_max_time):
        #Only ON states will be considered
        if(csv_states[i] != const_off_state):
            huffman.append(diftimes[i])
    #Morse data
    else:
        #Only ON states will be considered
        if(csv_states[i] != const_off_state):
            morse.append(diftimes[i])
        else:
            #Receive a OFF pulse that last more than 2 Morse units of time means the end of a Morse character transmision.
            if(diftimes[i] > (cfg_morse_unit_time_s*2) and len(morse)>0):
                morsechar(morse)
                morse = []
                #An OFF pulse that last more than 5 Morse units of time means a blank space
                if(diftimes[i] > (cfg_morse_unit_time_s*5)):
                    morsestring.append(' ')

        #Receive a Morse data means the end of a possible Huffman character transmision
        if(len(huffman)>0):
            huffmanchar(huffman)
            huffman = []

#We print the final strings for each method, Huffman and Morse
f= open("morsehuffman_translation.txt","w+")
f.write("huffman: %s\r\n" % huffmanstring)
f.write("morse: %s\r\n" % morsestring)
f.close()

    
