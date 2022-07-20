rule tome_nocase
{
  strings:
    $re = /Tom|Sawyer|Huckleberry|Finn/ nocase
  condition:
    $re
}
