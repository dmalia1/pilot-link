/* 
 * address/Record.java:
 *
 * Copyright (C) 1997, 1998, Kenneth Albanowski
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. *
 */

package Pdapilot.address;

import java.io.*;

/** A representation of an address database record.
 */

public class Record extends Pdapilot.Record {

		public int[] phoneLabel;
		public int showPhone;
		public String[] entry;
		
		public Record() {
			super();
		}
		
		public Record(byte[] contents, Pdapilot.RecordID id, int index, int attr, int cat) {
			super(contents, id, index, attr, cat);
		}
		
		public native void unpack(byte[] data);
		public native byte[] pack();
        		
		public void fill() {
			entry = new String[19];
			phoneLabel = new int[5];
			showPhone = 0;
		}
		
        public String describe() {
        	StringBuffer c = new StringBuffer("phoneLabel=[");
        	for(int i=0;i<phoneLabel.length;i++) {
        		if (i>0)
        			c.append(",");
        		c.append(phoneLabel[i]);
        	}
        	c.append("], entry=[");
        	for(int i=0;i<entry.length;i++) {
        		if (i>0)
        			c.append(",");
        		c.append((entry[i] == null) ? "(null)" : entry[i]);
        	}
        	c.append("]");
          return "showPhone="+showPhone+", "+c+", "+super.describe();
        }
}
