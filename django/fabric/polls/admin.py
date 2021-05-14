from django.contrib import admin
from .models import Question
from .models import Choice 
from .models import Poller 
from .models import AnswerText
from .models import AnswerChoice

# Register your models here.
admin.site.register(Question)
admin.site.register(Choice)
admin.site.register(Poller)
admin.site.register(AnswerText)
admin.site.register(AnswerChoice)

class ItemAdmin(admin.ModelAdmin):
	exclude=("start_date",)
	readonly_fields=('Start_date', )
