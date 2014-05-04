import sqlite3
conn = sqlite3.connect("dicttrans.db")
c = conn.cursor()
c.execute("CREATE TABLE English (word VARCHAR(24) PRIMARY KEY, def TEXT)")
c.close()
conn.close()
