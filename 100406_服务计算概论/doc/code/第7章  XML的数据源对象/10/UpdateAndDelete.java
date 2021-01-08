import org.w3c.dom.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import javax.xml.parsers.*;
import java.io.File;

public class UpdateAndDelete {
    public static void main(String[] args) {
        Boolean flag;
        Document document = null;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            document = builder.parse(new File("02.xml"));
            document.normalize();
            Node root = document.getDocumentElement();
            /* ���root����Ԫ�� */
            if (root.hasChildNodes()) {
                /*���student�ڵ��б� */
                NodeList studentnodes = root.getChildNodes();
                /* ѭ��ȡ��student���нڵ� */
                for (int i = 0; i < studentnodes.getLength(); i++) {
                    NodeList studentlist = studentnodes.item(i).getChildNodes();
                    for (int k = 0; k < studentlist.getLength(); k++) {
                        Node subnode = studentlist.item(k);
                        /* �޸�nameֵ */
                        if (subnode.getNodeType() == Node.ELEMENT_NODE && subnode.getFirstChild().getNodeValue().equals("����"))
                        {
                            subnode.getFirstChild().setNodeValue("����");
                        }
                         /* ɾ���ڵ� */
                        if (subnode.getNodeType() == Node.ELEMENT_NODE && subnode.getFirstChild().getNodeValue().equals("����")) {
                            root.removeChild(studentnodes.item(i));
                        }
                    }
                }
            }

            /* �޸���󱣴� */
            flag = SaveXmlFile(document, "02.xml");
            if (flag) {
                System.out.println("�����ɹ���");
            } else {
                System.out.println("����ʧ�ܣ�");
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static boolean SaveXmlFile(Document document, String filename) {
        boolean flag = true;
        try {
            /* ��document�е�����д���ļ���  */
            TransformerFactory tFactory = TransformerFactory.newInstance();
            Transformer transformer = tFactory.newTransformer();
            DOMSource source = new DOMSource(document);
            StreamResult result = new StreamResult(new File(filename));
            transformer.transform(source, result);
        } catch (Exception ex) {
            flag = false;
            ex.printStackTrace();
        }
        return flag;
    }
}
