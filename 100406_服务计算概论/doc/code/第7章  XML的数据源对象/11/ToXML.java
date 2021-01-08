import org.w3c.dom.*;
import javax.xml.parsers.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import java.sql.*;
import java.io.File;

public class ToXML {
    public static void main(String[] args) {
        String   url="jdbc:odbc:myds";
        String   user="sa";
        String   password="sa";
        Connection conn ;
        try{
            Class.forName("sun.jdbc.odbc.JdbcOdbcDriver").newInstance();
            conn=   DriverManager.getConnection(url,user,password);
            Statement st = conn.createStatement();
            ResultSet rst = st.executeQuery("select * from students");
            if(rst!=null){
                writeXML(rst);
            }else{
                System.out.println("���ݿ���û�����ݣ�");
            }
        }catch(Exception e){
            e.printStackTrace();
        }

    }
    public static void writeXML(ResultSet rst) {
        ResultSet set = rst;
        boolean flag;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();

            Document document = builder.newDocument();
            /* ����students���ڵ� */
            Element rootElement = document.createElement("students");
            document.appendChild(rootElement);

             while(set.next()){
            Element sElement = document.createElement("student");
            rootElement.appendChild(sElement);


            /* Ϊstudent�ڵ�����ӽڵ� */
            Element element = document.createElement("name");
            element.appendChild(document.createTextNode(set.getString("name")));
            sElement.appendChild(element);

            element = document.createElement("sex");
            element.appendChild(document.createTextNode(set.getString("sex")));
            sElement.appendChild(element);

            element = document.createElement("age");
            element.appendChild(document.createTextNode(set.getString("age")));
            sElement.appendChild(element);
            }
            /* �����ļ� */
            flag = SaveXmlFile(document, "student.xml");
            if(flag){
                System.out.println("�ļ������ɹ���");
            }else{
                System.out.println("�ļ�����ʧ�ܣ�");
            }
        }
        catch (Exception ex) {
            ex.printStackTrace();
        }
    }

   public static boolean SaveXmlFile(Document document,String filename)
   {
      boolean flag = true;
      try
      {
            /* ��document�е�����д���ļ���  */
            TransformerFactory tFactory = TransformerFactory.newInstance();
            Transformer transformer = tFactory.newTransformer();
            DOMSource source = new DOMSource(document);
            StreamResult result = new StreamResult(new File(filename));
            transformer.transform(source, result);
        }catch(Exception ex)
        {
            flag = false;
            ex.printStackTrace();
        }
        return flag;
   }
}
