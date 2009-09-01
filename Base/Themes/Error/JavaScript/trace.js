$(document).ready(function(){
  $('div.popup a.close').click(function(){
    $(this).parent('div').hide();
    return false;
  });
  
  $('a.arg').click(function(e){
    var id = $(this).attr('id');
    var popupid = '#' + id + 'Popup';
    $(popupid).css('margin-top', e.pageY + 5);
    $(popupid).css('margin-left', e.pageX);
    $(popupid).show();
    return false;
  });
  
});