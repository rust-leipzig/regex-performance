rule tom4
{
  strings:
    $re = /.{2,4}(Tom|Sawyer|Huckleberry|Finn)/
  condition:
    $re
}
