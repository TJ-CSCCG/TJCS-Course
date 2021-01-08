<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
  <xsl:element name="html">
     <xsl:element name="body">
        <xsl:element name="table">
            <xsl:attribute name="border">1</xsl:attribute>
            <xsl:element name="tr">
               <xsl:attribute name="bgcolor">yellow</xsl:attribute>
               <xsl:element name="td">公司名称</xsl:element>
               <xsl:element name="td">公司URL</xsl:element>
               <xsl:element name="td">图片</xsl:element>
            </xsl:element>
            <xsl:element name="tr">
               <xsl:element name="td">
                    <xsl:value-of select="root/company/name" />
               </xsl:element>
               <xsl:element name="td">
                  <xsl:element name="a">
                     <xsl:attribute name="href">
                          <xsl:value-of select="root/company/url" />
                     </xsl:attribute>
                     <xsl:value-of select="root/company/url" />
                  </xsl:element>
               </xsl:element>
               <xsl:element name="td">
                  <xsl:element name="image">
                     <xsl:attribute name="src">
                         <xsl:value-of select="root/company/picture" />
                     </xsl:attribute>
                  </xsl:element>
               </xsl:element>
            </xsl:element>
        </xsl:element>
     </xsl:element>
  </xsl:element>
</xsl:template>
</xsl:stylesheet>