import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo4 {
     public static void main(String[] args) {
         int m=0;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("03.xml"));

            Element root = document.getDocumentElement();
            System.out.println(root.getNodeName()+"�ڵ���ӽڵ��У�");

            NodeList nodeList = root.getChildNodes();
            for(int i=0;i<nodeList.getLength();i++){
                Node node = nodeList.item(i);
                   if(node.getNodeType() == Node.TEXT_NODE){
                        Text tNode = (Text)node;
                        System.out.println("��"+(++m)+"���ڵ���Text���ͽڵ㣬�ڵ����ݣ�");
                        System.out.print(tNode.getWholeText());
                    }
                    if(node.getNodeType() == Node.CDATA_SECTION_NODE){
                        CDATASection cdataNode = (CDATASection)node;
                        System.out.println("��"+(++m)+"���ڵ���CDATASection���ͽڵ㣬�ڵ����ݣ�");
                        System.out.print(cdataNode.getWholeText());
                    }
                }
             System.out.println("����"+m+"���ڵ㡣");
            } catch (Exception e) {
            e.printStackTrace();
        }

    }
}
