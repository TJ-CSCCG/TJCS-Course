import org.w3c.dom.*;
import javax.xml.parsers.*;
import java.io.*;
import java.sql.*;


public class ToDB {
    public static void main(String[] args) {
        String url ="jdbc:odbc:myds";
        String user = "sa";
        String password = "sa";
        Connection conn;
        String[][] students = new String[4][3];
        try {

            Class.forName("sun.jdbc.odbc.JdbcOdbcDriver").newInstance();
            conn = DriverManager.getConnection(url, user, password);
            
            DocumentBuilderFactory domfac = DocumentBuilderFactory.newInstance();
            DocumentBuilder dombuilder = domfac.newDocumentBuilder();

            Document document = dombuilder.parse(new File("03.xml"));

            NodeList namelist = document.getElementsByTagName("name");
            for (int i = 0; i < namelist.getLength(); i++) {
                Node node = namelist.item(i);
                String name = node.getTextContent().trim();
                students[i][0] = name;
            }
            NodeList sexlist = document.getElementsByTagName("sex");
            for (int i = 0; i < sexlist.getLength(); i++) {
                Node node = sexlist.item(i);
                String sex = node.getTextContent().trim();
                students[i][1] = sex;
            }
            NodeList agelist = document.getElementsByTagName("age");
            for (int i = 0; i < agelist.getLength(); i++) {
                Node node = agelist.item(i);
                String age = node.getTextContent().trim();
                students[i][2] = age;
            }

            PreparedStatement pst = conn.prepareStatement("insert into students(name,sex,age) values(?,?,?)");
            for (int i = 0; i < students.length; i++) {
                pst.setString(1, students[i][0]);
                pst.setString(2, students[i][1]);
                pst.setInt(3, Integer.parseInt(students[i][2]));
                pst.executeUpdate();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}



