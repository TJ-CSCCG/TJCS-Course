# -*- coding: utf-8 -*-

import MySQLdb
import sys


# 打开数据库连接 参数依次为host, user, password, database
db = MySQLdb.connect("localhost", "root", "123456", "course_ex", charset='utf8')

try:
    # 使用cursor()方法获取操作游标
    cursor = db.cursor()

    # 使用execute方法执行SQL语句
    # 查询User表中的数据
    cursor.execute("SELECT * from User")
    # 使用 fetchall() 方法获取所有数据
    data = cursor.fetchall()
    # 输出每一行的数据
    for row in data:
        row_data = []
        for c in row:
            row_data.append(str(c))
        print (' '.join(row_data))

    # 将编号为0000000001的用户的年龄修改成40岁
    uno = u'0000000001'
    age = 40
    cursor.execute("UPDATE User SET Uage=%d where Uno=%s" % (age, uno))
    print ('修改成功')

    # 显示修改后的结果
    cursor.execute("SELECT * from User")
    data = cursor.fetchall()
    for row in data:
        row_data = []
        for c in row:
            row_data.append(str(c))
        print (' '.join(row_data))

except:
    # 若连接失败或语句执行失败，输出错误原因
    import traceback
    traceback.print_exc()

# 关闭数据库连接
db.commit()
db.close()
