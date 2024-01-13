import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class Query {
    public static void main(String[] args) {
        Document document = null;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(new File("02.xml"));
            document.normalize();
            Node root = document.getDocumentElement();
           /** 如果root有子元素 */
            if (root.hasChildNodes()) {
                /** studentnodes */
                NodeList studentnodes = root.getChildNodes();
                /** 循环取得student所有节点 */
                for (int i = 0; i < studentnodes.getLength(); i++) {
                    NodeList studentlist = studentnodes.item(i).getChildNodes();
                    for (int k = 0; k < studentlist.getLength(); k++) {
                        Node subnode = studentlist.item(k);
                        if (subnode.getNodeType() == Node.ELEMENT_NODE) {
                            /** 打印student所有节点属性的值 */
                            System.out.print(subnode.getNodeName() + ":" + subnode.getFirstChild().getNodeValue()+"；");
                        }
                    }
                    System.out.println();
                }
            }
        }catch (Exception e) {
            e.printStackTrace();
        }
    }

}
