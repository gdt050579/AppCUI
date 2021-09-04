Layout in AppCUI
================

Each control in AppCUI is created based on a layout rule that can be described as an ascii string that respects the following format:

.. code-block:: c

   "key:value,key:value,...key:value"

Where key can be one of the following:

.. hlist::
   :columns: 1

   * **x** --> "X" coordonate
   * **y** --> "Y" coordonate
   * **w** or **width** --> the width of the control
   * **h** or **height** --> the height of the control
   * **a** or **align** --> the alignament of the control with respect to the (X,Y) coordonates

Except for **align** key, the rest of the parameters can have two types of values:

.. hlist::
   :columns: 1

   * **numerical** --> an integer (positive or negative) value : Ex:``x:100`` --> X will be 100
   * **percentage** --> a floating (positive or negative) value that represent a percentage from the controls parent sizes : Ex:``x:50%,y:50%`` means (X,Y) will be at the center of current control's parent

**align** key has the following values:

.. hlist::
   :columns: 1
                   
   * **tl** or **lt** or **top-left**
   * **tc** or **ct** or **top-center** 
   * **tr** or **rt** or **top-right**
   * **rc** or **cr** or **right-center** 
   * **rb** or **br** or **bottom-right** 
   * **bc** or **cb** or **bottom-center** 
   * **lb** or **bl** or **bottom-left** 
   * **lc** or **cl** or **right-center** 
   * **c** or **center**

Examples:
---------

+--------------------------------+------------------------------------------------------------------------+
| Layout                         | Result (2)                                                             |
+================================+========================================================================+
| **x:5,y:5,w:10,h:10** or       |If no alignament is provided, top-left will be considered as a default. |
|                                |                                                                        |
| **x:5,y:5,w:10,h:4,a:tl**      | .. image:: pics/layout_xywh_tl.png                                     |
+--------------------------------+------------------------------------------------------------------------+
| **x:30,y:20,w:10,h:4,a:br**    | .. image:: pics/layout_xywh_br.png                                     |
+--------------------------------+------------------------------------------------------------------------+
| **x:50%,y:50%,w:10,h:3,a:c**   | .. image:: pics/layout_xywh_c.png                                      |
+--------------------------------+------------------------------------------------------------------------+
| **x:50%,y:50%,w:100%,h:3,a:c** | .. image:: pics/layout_xywh_c_full_width.png                           |
+--------------------------------+------------------------------------------------------------------------+
