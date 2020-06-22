#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <mysql.h> 
#include <iostream>

#include <vector>
using namespace std;
typedef vector<char *> ColName; 
int main()
{
	/*-------------------------------------
	 *   存储要连接数据库的必要信息       *
	--------------------------------------*/

    const char user[] = "root";          //username
    const char pswd[] = "940923";        //password
    const char host[] = "localhost";     //or"127.0.0.1"
    const char table[] = "course_ex";        //database
    unsigned int port = 3306;            //server port        

    //建立一个mysql对象
	MYSQL mytest;
	//初始化数据库对象
	mysql_init(&mytest);
		
	ColName array1;

    int res;

    if(mysql_real_connect(&mytest ,host ,user ,pswd ,table ,port ,NULL ,0))
		//连接数据库并且在成功连接的情况下进行操作
    {
        cout<<"连接" << host << "-" << table << "成功" <<endl;

        mysql_query(&mytest, "SET NAMES GBK"); //设置编码格式

		string tmpquery = "";

		char query[100] = "";

		MYSQL_ROW sql_row;//保存行信息

		MYSQL_FIELD *fd;//保存列信息
    
		do
		{
			cout << "输入 mysql 语句 :" << endl;
			getline(cin ,tmpquery);

			//请求语句的输入
			int i = 0 ,j = 0;
			while(i < tmpquery.length())
			{
				query[i] = tmpquery[i];
				i ++;
			}

			//保存请求结果的变量
			MYSQL_RES *dynamic_query;

			res = mysql_query(&mytest ,query);						
			if(!res)
			{
				cout << "请求已成功执行" << endl;
				if(tmpquery.substr(0,6) == "select")
				{
					dynamic_query = mysql_store_result(&mytest);
					if(dynamic_query)
					{					
						for(i = 0 ;fd = mysql_fetch_field(dynamic_query) ;i++)//获取列名(类似移动游标)
							array1.push_back(fd->name);

						j = mysql_num_fields(dynamic_query);   

						while(sql_row = mysql_fetch_row(dynamic_query))//获取具体的数据
						{
							for(i = 0;i < j; i++)
							{
								cout << array1[i]  << "-" << sql_row[i] << endl;
                                从mysql请求返回的结果都是以字符串形式保存的
								string tmptest = sql_row[i];

							}                   
						}
						if(dynamic_query != NULL) 
							mysql_free_result(dynamic_query);
					}
					else					
						cout << "查询数据出错" << endl;		
				}
				else
				{
					cout << "操作完成!" << endl;
				}				
			}
			else if(query[0] == '0')
			{
				cout << "即将退出!" << endl;
				break;
			}
			else
			{
				cout << "请求执行失败" << endl;
			}
			res = 0;
		}while(1);
	}
	else
		cout << "数据库连接失败,即将退出！" << endl;
    mysql_close(&mytest);//断开连接
    return 0;
}
