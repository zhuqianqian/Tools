import sys
import sqlite3
import urllib.request

condition = '"'+sys.argv[1] + '%"'
print(condition)
sql = 'SELECT WORD FROM English WHERE WORD LIKE '+condition
conn = sqlite3.connect('dict.db')
c = conn.cursor()
c.execute(sql)
wordlist = c.fetchall()
for row in wordlist:
    word = row[0]
    url = 'http://dict.youdao.com/search?q='+word
    file = open(sys.argv[1]+'\\'+word, "wb")
    request = urllib.request.Request(url)
    response = urllib.request.urlopen(request)
    html = response.read()
    file.write(html)
    file.close()
    print('Retrieved: ' + word + '\n')
