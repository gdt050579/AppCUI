Layout in AppCUI
================

Each control in AppCUI is created based on a layout rule that can be described as an ascii string that respects the following format:

.. code-block:: c

   "key:value,key:value,...key:value"

Where key can be one of the following:

+---------+----------+--------------------------+----------------------------------------------------------+
|| Key    || Alias   || Value type              || Description                                             |
||        || (short) ||                         ||                                                         |
+=========+==========+==========================+==========================================================+
| x       |          | numerical or percentage  | "X" coordonate                                           |
+---------+----------+--------------------------+----------------------------------------------------------+
| y       |          | numerical or percentage  | "Y" coordonate                                           |
+---------+----------+--------------------------+----------------------------------------------------------+
|| left   || l       || numerical or percentage || left anchor for the control                             |
||        ||         ||                         || (the space between parent left margin and control)      |
+---------+----------+--------------------------+----------------------------------------------------------+
|| right  || r       || numerical or percentage || right anchor for the control                            |
||        ||         ||                         || (the space between parent right margin and control)     |
+---------+----------+--------------------------+----------------------------------------------------------+
|| top    || t       || numerical or percentage || top anchor for the control                              |
||        ||         ||                         || (the space between parent top margin and control)       |
+---------+----------+--------------------------+----------------------------------------------------------+
|| bottom || b       || numerical or percentage || bottom anchor for the control                           |
||        ||         ||                         || (the space between parent bottom margin and control)    |
+---------+----------+--------------------------+----------------------------------------------------------+
| width   | w        | numerical or percentage  | the width of the control                                 |
+---------+----------+--------------------------+----------------------------------------------------------+
| height  | h        | numerical or percentage  | the height of the control                                |
+---------+----------+--------------------------+----------------------------------------------------------+
| dock    | d        | docking value            | the way the entire control is docked on its parent       |
+---------+----------+--------------------------+----------------------------------------------------------+
| align   | a        | alignament value         | the way the entire control is aligne against a fix point |
+---------+----------+--------------------------+----------------------------------------------------------+

**Remarks**: Key aliases can be use to provide a shorter format for a layout. In other words, the following two formats are identical: ``x:10,y:10,width:30,height:30`` and ``x:10,y:10,w:30,h:30``

A numerical value is represented by an integera (positive and negative) number between **-30000** and **30000**. Example: ``x:100`` --> X will be 100. Using a value outside accepted interval (**[-30000..30000]**) will reject the layout.

A percentage value is represented by a floating value (positive and negative) succeded by the character ``%`` between **-300%** and **300%**. Example: ``x:12.75%`` --> X will be converted to a numerical value that is equal to the width of its parent multiplied by ``0.1275``. Using a value outside accepted interval (**[-300%..300%]**) will reject the layout. Percentage values can be use to ensure that if a parent size is changed, its children change their size with it.


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
