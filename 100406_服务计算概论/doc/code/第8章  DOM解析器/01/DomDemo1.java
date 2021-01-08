import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo1 {
    public static void main(String[] args) {
        try{
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("01.xml"));

            String version = document.getXmlVersion();
            String encoding = document.getXmlEncoding();
            System.out.println("�ļ�01.xml�����İ汾���ǣ�"+version);
            System.out.println("�ļ�01.xml�����ı����ǣ�"+encoding);

            Element root = document.getDocumentElement();
            System.out.println("�ļ�01.xml�ĸ��ڵ��ǣ�"+root.getNodeName());

            NodeList nodelist = document.getElementsByTagName("����");
            for (int i = 0; i < nodelist.getLength(); i++) {
                Node node = nodelist.item(i);
                System.out.print(node.getNodeName());
                System.out.println(node.getTextContent());
            }

        }catch(Exception e){
            e.printStackTrace();
        }

    }
}
