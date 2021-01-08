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
           /** ���root����Ԫ�� */
            if (root.hasChildNodes()) {
                /** studentnodes */
                NodeList studentnodes = root.getChildNodes();
                /** ѭ��ȡ��student���нڵ� */
                for (int i = 0; i < studentnodes.getLength(); i++) {
                    NodeList studentlist = studentnodes.item(i).getChildNodes();
                    for (int k = 0; k < studentlist.getLength(); k++) {
                        Node subnode = studentlist.item(k);
                        if (subnode.getNodeType() == Node.ELEMENT_NODE) {
                            /** ��ӡstudent���нڵ����Ե�ֵ */
                            System.out.print(subnode.getNodeName() + ":" + subnode.getFirstChild().getNodeValue()+"��");
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
