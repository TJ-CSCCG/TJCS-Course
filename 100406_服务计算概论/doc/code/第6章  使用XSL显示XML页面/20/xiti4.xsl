<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
  <xsl:element name="html">
     <xsl:element name="body">
        <xsl:element name="table">
            <xsl:attribute name="border">1</xsl:attribute>
            <xsl:attribute name="width">300</xsl:attribute>
		<xsl:element name="tr">
			<xsl:element name="td">
                          <xsl:attribute name="align">center</xsl:attribute>
                          name
                        </xsl:element>
			<xsl:element name="td">
                          <xsl:attribute name="align">center</xsl:attribute>
                          lxfs
                        </xsl:element>
		</xsl:element>
		<xsl:for-each select="root/persion">
		<xsl:element name="tr">
			<xsl:apply-templates />
		</xsl:element>
		</xsl:for-each>
		</xsl:element>
     </xsl:element>
  </xsl:element>
</xsl:template>
<xsl:template match="//name">
<xsl:element name="td"><xsl:value-of  select=".[@id$eq$'2']" /></xsl:element>
</xsl:template>
<xsl:template match="//lxfs">
<xsl:element name="td"><xsl:value-of  select="tel" /></xsl:element>
</xsl:template>
</xsl:stylesheet>